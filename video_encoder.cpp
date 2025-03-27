#include "video_encoder.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstring>
#include <fstream>

namespace VideoStreamer
{
    VideoEncoder::VideoEncoder(const AppConfig &cfg) : config(cfg) {}

    void VideoEncoder::encode(const std::vector<std::string> &inputFiles, const std::string &outputFile)
    {
        std::string dir = config.tempDir;
        if (!dir.empty() && dir.back() != '/')
        {
            dir += '/';
        }

        // 检查 tempDir 是否存在，不存在则创建
        if (mkdir(config.tempDir.c_str(), 0755) != 0)
        {
            if (errno != EEXIST)
            { // 如果目录创建失败且不是因为已经存在的原因
                std::cerr << "Failed to create directory: " << strerror(errno) << std::endl;
            }
        }
        // 将输入列表写入临时文件
        const std::string listFile = dir + "ffmpeg_list.txt";
        {
            std::ofstream f(listFile);
            for (const auto &file : inputFiles)
            {
                f << "file '" << file << "'\n"; // 正确格式
            }
        }

        // 创建子进程进行FFmpeg编码操作
        pid_t pid = fork(); // 创建子进程
        if (pid == 0)       // 子进程执行编码操作
        {
            if (config.rtmpUrl.empty())
            {
                // 仅生成H264文件
                execlp(config.ffmpegPath.c_str(), "ffmpeg",
                       "-y",
                       "-f", "concat",
                       "-safe", "0",
                       "-i", listFile.c_str(),
                       "-c:v", "libx264",
                       "-b:v", "3M",
                       "-g", "15",
                       "-profile:v", "high422",
                       "-f", "h264",
                       outputFile.c_str(),
                       nullptr);
            }
            else
            {
                // 同时输出到文件和RTMP
                // std::string teeOutput = "[f=h264]" + outputFile + "|[f=flv]" + config.rtmpUrl;
                // execlp(config.ffmpegPath.c_str(), "ffmpeg",
                //        "-y", "-loglevel", "debug",
                //        "-f", "concat",
                //        "-safe", "0",
                //        "-i", listFile.c_str(),
                //        "-c:v", "libx264",
                //        "-b:v", "3M",
                //        "-g", "15",
                //        "-profile:v", "high422",
                //        "-map", "0:v", "-an",
                //        "-f", "tee",
                //        teeOutput.c_str(),
                //        nullptr);

                // 仅输出到RTMP
                execlp(config.ffmpegPath.c_str(), "ffmpeg",
                       "-y",
                       "-f", "concat",
                       "-safe", "0",
                       "-i", listFile.c_str(),
                       "-c:v", "libx264",
                       "-b:v", "3M",
                       "-g", "15",
                       "-profile:v", "high422",
                       "-map", "0:v", "-an",
                       "-f", "flv",
                       config.rtmpUrl.c_str(),
                       nullptr);
            }

            exit(EXIT_FAILURE); // 如果execlp失败，则退出子进程
        }
        int status;
        waitpid(pid, &status, 0);               // 父进程等待子进程执行完毕
        if (std::remove(listFile.c_str()) != 0) // 删除临时文件
        {
            std::cerr << "[VideoEncoder] 无法删除文件 '" << listFile << "': " << strerror(errno) << std::endl;
        }

        // 检查编码是否成功
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) != 0)
            {
                throw std::runtime_error("FFmpeg编码失败");
            }
        }

        // 执行保留策略
        switch (config.deletePolicy)
        {
        case DeletePolicy::KeepAll:
        {

            break;
        }

        case DeletePolicy::DeleteOnSuccess:
        {
            // 删除当前批次文件
            for (const auto &file : inputFiles)
            {
                const std::string frameFile = dir + file;
                if (std::remove(frameFile.c_str()) != 0)
                {
                    std::cerr << "[VideoEncoder] 无法删除文件 '" << frameFile << "': " << strerror(errno) << std::endl;
                }
                else
                {
                    std::cout << "[VideoEncoder] 编码成功，删除原始帧文件" << std::endl;
                }
            }
            break;
        }

        case DeletePolicy::DeleteWhenExceed:
        {
            // 将当前批次加入跟踪队列
            for (const auto &file : inputFiles)
            {
                FileInfo info;
                if (getFileInfo(file, info))
                {
                    fileQueue.push_back(info);
                    totalSize += info.size;
                }
            }

            // 计算内存阈值（字节）
            const size_t maxBytes = config.maxMemoryMB * 1024 * 1024;

            // 删除最早文件直到满足内存限制
            while (!fileQueue.empty() && totalSize > maxBytes)
            {
                const auto &oldest = fileQueue.front();
                if (std::remove(oldest.path.c_str()) == 0)
                {
                    totalSize -= oldest.size;
                    fileQueue.pop_front();
                    std::cout << "[VideoEncoder] 删除旧文件: " << oldest.path << "\n";
                }
                else
                {
                    break; // 防止无限循环
                }
            }
            break;
        }
        }
    }

    void VideoEncoder::encodeRealtime(const std::shared_ptr<ob::ColorFrame> &frame)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // 使用管道直接传递帧数据给FFmpeg
            execlp(config.ffmpegPath.c_str(), "ffmpeg",
                   "-y",
                   "-f", "rawvideo",
                   "-pix_fmt", "rgb24", // 根据实际帧格式调整
                   "-s", std::to_string(config.targetWidth) + "x" + std::to_string(config.targetHeight).c_str(),
                   "-i", "pipe:0", // 从标准输入读取
                   "-c:v", "libx264",
                   "-preset", "ultrafast",
                   "-tune", "zerolatency",
                   "-f", "flv",
                   config.rtmpUrl.c_str(),
                   nullptr);

            // 写入帧数据到标准输出
            write(STDOUT_FILENO, frame->data(), frame->dataSize());
            exit(EXIT_FAILURE);
        }
        // 注意：需要添加更完善的管道管理和错误处理
    }

    bool VideoEncoder::getFileInfo(const std::string &path, FileInfo &info)
    {
        struct stat statBuf;
        if (stat(path.c_str(), &statBuf) != 0)
            return false;

        info.path = path;
        info.size = statBuf.st_size;
        info.createdTime = std::chrono::system_clock::from_time_t(statBuf.st_ctime);
        return true;
    }
} // namespace VideoStreamer