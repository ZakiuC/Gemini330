#pragma once
#include "config.hpp"
#include <memory>
#include <string>

namespace VideoStreamer
{
    /**
     * OSSUploader类用于将文件上传到阿里云OSS
     */
    class OSSUploader
    {
    public:
        explicit OSSUploader(const AppConfig &cfg);
        /**
         * 上传文件的接口，传入文件路径
         */
        void uploadFile(const std::string &filePath);

    private:
        /**
         * 初始化OSS客户端
         */
        void initClient();

        /**
         * 验证文件是否存在
         */
        bool validateFile(const std::string &path);

        /**
         * 生成上传到OSS的对象名称
         */
        std::string generateObjectName();

        /**
         * 打开文件并返回文件流
         */
        std::shared_ptr<std::iostream> openFileStream(const std::string &path);

        /**
         * 执行文件上传操作
         */
        void executeUpload(const std::string &objectName, const std::shared_ptr<std::iostream> &stream);

        /**
         * 删除本地文件
         */
        void cleanupFile(const std::string &path);

        // 存储应用程序的配置
        AppConfig config;

        // OSS客户端，负责与阿里云OSS进行交互
        std::shared_ptr<AlibabaCloud::OSS::OssClient> client;
    };
} // namespace VideoStreamer