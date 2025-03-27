#include "oss_uploader.hpp"
#include <fstream>
#include <chrono>
#include <cstdio>
#include <unistd.h>

namespace VideoStreamer
{
    OSSUploader::OSSUploader(const AppConfig &cfg) : config(cfg)
    {
        initClient();
    }

    /**
     * 初始化OSS客户端
     */
    void OSSUploader::initClient()
    {
        // 配置OSS客户端的一些基本设置，例如最大连接数
        AlibabaCloud::OSS::ClientConfiguration ossConfig;
        ossConfig.maxConnections = 10;
        ossConfig.connectTimeoutMs = config.connectTimeoutMs;
        ossConfig.requestTimeoutMs = config.requestTimeoutMs;

        // 创建OSS客户端对象，使用配置中的访问凭证
        client = std::make_shared<AlibabaCloud::OSS::OssClient>(
            config.endpoint,        // OSS访问域名
            config.accessKeyId,     // AccessKeyId
            config.accessKeySecret, // AccessKeySecret
            ossConfig);             // 客户端配置
    }

    bool OSSUploader::validateFile(const std::string &path)
    {
        // 使用access函数检查文件是否存在
        return access(path.c_str(), F_OK) != -1;
    }

    std::string OSSUploader::generateObjectName()
    {
        return config.uploadPrefix + // 上传路径前缀（例如："live/"）
               std::to_string(
                   std::chrono::high_resolution_clock::now() // 当前时间戳
                       .time_since_epoch()
                       .count()) +
               ".h264"; // 文件后缀为.h264
    }

    std::shared_ptr<std::iostream> OSSUploader::openFileStream(const std::string &path)
    {
        // 创建文件流对象，打开文件进行二进制读取
        auto stream = std::make_shared<std::fstream>(
            path, std::ios::in | std::ios::binary);

        // 检查文件是否成功打开
        if (!stream->is_open())
        {
            throw std::runtime_error("[OSSUploader] 文件打开失败: " + path); // 文件打开失败，抛出异常
        }
        return stream; // 返回文件流
    }

    void OSSUploader::executeUpload(const std::string &objectName, const std::shared_ptr<std::iostream> &stream)
    {
        std::cout << "[OSSUploader] used objectName: " << objectName << std::endl;

        // 创建上传请求，指定存储桶、对象名称和文件流
        AlibabaCloud::OSS::PutObjectRequest request(config.bucket, objectName, stream);

        // 执行上传请求
        auto outcome = client->PutObject(request);

        // 检查上传是否成功，若失败则抛出异常
        if (!outcome.isSuccess())
        {
            throw std::runtime_error("[OSSUploader] OSS Error: " + outcome.error().Message());
        }
    }

    void OSSUploader::cleanupFile(const std::string &path)
    {
        // 删除本地文件
        std::remove(path.c_str());
    }

    void OSSUploader::uploadFile(const std::string &filePath)
    {
        // 检查文件是否存在
        if (!validateFile(filePath))
        {
            std::cerr << "[OSSUploader] 文件不存在: " << filePath << std::endl; // 如果文件不存在，输出错误信息
            return;
        }

        try
        {
            // 生成上传对象的名称
            auto objectName = generateObjectName();

            // 打开文件流
            auto fileStream = openFileStream(filePath);

            // 执行文件上传
            executeUpload(objectName, fileStream);

            // 上传完成后删除本地文件
            cleanupFile(filePath);
            std::cout << "[OSSUploader] 上传成功后删除本地文件：" << filePath << std::endl;
        }
        catch (const std::exception &e)
        {
            // 捕获并输出上传过程中的错误
            std::cerr << "[OSSUploader] 上传失败: " << e.what() << std::endl;
        }
    }
} // namespace VideoStreamer