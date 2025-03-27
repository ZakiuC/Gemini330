#pragma once
#include "config.hpp"
#include <vector>
#include <string>

namespace VideoStreamer
{
    // 文件信息结构体
    struct FileInfo {
        std::string path;
        size_t size;
        std::chrono::system_clock::time_point createdTime;
    };
    
    /**
     * VideoEncoder 类用于视频编码
     */
    class VideoEncoder
    {
    public:
        explicit VideoEncoder(const AppConfig &cfg);

        /**
         * 编码函数，将多个输入文件编码成一个输出文件
         */
        void encode(const std::vector<std::string> &inputFiles,
                    const std::string &outputFile);

    private:
        /**
         * 获取文件信息
         */
        bool getFileInfo(const std::string& path, FileInfo& info);

        // 配置对象，存储编码所需的配置信息
        AppConfig config;
        std::deque<FileInfo> fileQueue;  // 文件跟踪队列
        size_t totalSize = 0;            // 当前总文件大小
    };
} // namespace VideoStreamer