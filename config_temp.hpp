#pragma once
#include <string>
#include <alibabacloud/oss/OssClient.h>
#include <libobsensor/ObSensor.hpp>

namespace VideoStreamer
{
    /**
     * 帧文件保留策略
     */
    enum class DeletePolicy {
        KeepAll = 0,        // 不删除
        DeleteOnSuccess = 1,// 删除成功编码的
        DeleteWhenExceed = 2// 超过内存限制时删除
    };
    
    /**
     * 配置结构体，存储应用程序的所有配置信息
     */
    struct AppConfig
    {
        // 摄像头参数
        int targetWidth = 1280;   // 摄像头目标宽度，默认为1280像素
        int targetHeight = 720;   // 摄像头目标高度，默认为720像素
        int targetFPS = 15;       // 摄像头目标帧率，默认为15帧每秒 
        ob_format colorFormat = OB_FORMAT_MJPG;  // 摄像头颜色格式，默认为MJPEG格式

        // 编码参数
        int h264GroupSize = 8;    // H.264编码的关键帧间隔，默认为8
        std::string ffmpegPath = "ffmpeg";  // FFmpeg的路径，默认为"ffmpeg"
        bool isDeleteOnSuccess = true;  // 在编码成功后是否删除原始帧文件

        // OSS（阿里云对象存储）参数
        std::string bucket = "your-bucket-name";  // OSS存储桶名称
        std::string endpoint = "your-endpoint";  // OSS访问域名
        std::string accessKeyId = "your-AccessKey-id";  // OSS的AccessKey ID
        std::string accessKeySecret = "your-AccessKey-Secret";  // OSS的AccessKey Secret
        std::string uploadPrefix = "live/";  // 上传到OSS的前缀路径，默认为"live/"
        long connectTimeoutMs = 2000; // 连接超时
        long requestTimeoutMs = 2000;  // 请求超时

        // 系统参数
        int uploadThreads = 2;  // 上传线程数，默认为2个线程
        int maxQueueSize = 20;  // 最大队列大小，默认为20
        std::string tempDir = "./tmp/";  // 临时文件夹路径，默认为"/tmp/"
        DeletePolicy deletePolicy = DeletePolicy::DeleteOnSuccess;  // 删除策略配置
        size_t maxMemoryMB = 1024;    // 内存阈值（单位：MB）
    };
} // namespace VideoStreamer
