#include "stream_processor.hpp"
#include "oss_uploader.hpp"
#include <chrono>
#include <cstdio>
#include <thread>
#include <fstream>

namespace VideoStreamer
{
    StreamProcessor::StreamProcessor(const AppConfig &cfg)
        : config(cfg),
          camera(cfg),
          encoder(cfg),
          frameQueue(std::make_shared<ThreadSafeQueue<std::string>>()) {}

    void StreamProcessor::start()
    {
        running = true;        // 设置为运行状态
        setupUploadWorkers();  // 设置上传工作线程
        startProcessingLoop(); // 启动视频处理循环
    }

    void StreamProcessor::stop()
    {
        running = false; // 设置为停止状态
        cleanup();       // 清理资源
    }

    void StreamProcessor::setupUploadWorkers()
    {
        for (int i = 0; i < config.uploadThreads; ++i) // 根据配置启动多个上传线程
        {
            workers.emplace_back(
                [this]()
                {
                    OSSUploader uploader(config); // 创建OSS上传对象
                    while (running)
                    { // 保持上传线程运行状态
                        processUpload(uploader);
                    }
                });
        }
    }

    void StreamProcessor::processUpload(OSSUploader &uploader)
    {
        auto file = uploadQueue.pop(); // 从上传队列中取出文件
        if (!file.empty())
        {
            std::cout << "[StreamProcessor] Uploading file: " << file << std::endl; // 打印出待上传文件的路径
            uploader.uploadFile(file);                                              // 执行上传操作
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void StreamProcessor::startProcessingLoop()
    {
        int frameCounter = 0; // 帧计数器
        while (running)
        {
            if (auto frame = camera.getFrame()) // 获取新的视频帧
            {
                handleNewFrame(frame, frameCounter); // 处理视频帧
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 控制帧获取的频率
        }
    }

    void StreamProcessor::handleNewFrame(std::shared_ptr<ob::ColorFrame> frame, int &counter)
    {
        try
        {
            auto tmpFile = saveTempFrame(frame); // 保存帧为临时文件
            manageFrameQueue(tmpFile);           // 管理帧队列
            checkBatchEncoding(++counter);       // 检查是否需要进行批量编码
        }
        catch (const std::exception &e)
        {
            std::cerr << "[StreamProcessor] 帧处理错误: " << e.what() << std::endl; // 捕获并输出异常
        }
    }

    std::string StreamProcessor::saveTempFrame(std::shared_ptr<ob::ColorFrame> frame)
    {
        // 确保目录以斜杠结尾
        std::string dir = config.tempDir;
        if (!dir.empty() && dir.back() != '/')
        {
            dir += '/';
        }

        char filename[128];
        char savePath[128];
        snprintf(filename, sizeof(filename), "frame_%ld.jpg",
                 std::chrono::high_resolution_clock::now()
                     .time_since_epoch()
                     .count());
        snprintf(savePath, sizeof(savePath), "%s%s", config.tempDir.c_str(), filename);

        std::ofstream file(savePath, std::ios::binary);                               // 打开文件进行二进制写入
        file.write(reinterpret_cast<const char *>(frame->data()), frame->dataSize()); // 写入帧数据
        return filename;                                                              // 返回文件名
    }

    void StreamProcessor::manageFrameQueue(const std::string &filename)
    {
        frameQueue->push(filename);                   // 将文件名加入队列
        if (frameQueue->size() > config.maxQueueSize) // 如果队列超过最大长度
        {
            std::remove(frameQueue->pop().c_str()); // 删除队列中的旧文件
        }
    }

    void StreamProcessor::checkBatchEncoding(int counter)
    {
        if (counter % config.h264GroupSize == 0) // 每当帧数达到指定数量时进行批量编码
        {
            processBatchEncoding(); // 执行批量编码
        }
    }

    void StreamProcessor::processBatchEncoding()
    {
        std::vector<std::string> batch; // 存储一批待编码的文件
        while (true)
        {
            auto file = frameQueue->pop(); // 从队列中取出文件
            if (file.empty())              // 如果队列为空，则退出
                break;
            batch.push_back(file); // 将文件加入批次
        }

        if (!batch.empty()) // 如果批次非空
        {
            char outputFile[128];
            snprintf(outputFile, sizeof(outputFile), "%sout_%ld.h264", // 生成输出文件名
                     config.tempDir.c_str(),
                     std::chrono::high_resolution_clock::now()
                         .time_since_epoch()
                         .count());
            
            std::cout << "[StreamProcessor] Pushing file to uploadQueue: " << outputFile << std::endl; // 打印推送文件名
            encoder.encode(batch, outputFile);                                                         // 执行编码

            uploadQueue.push(outputFile); // 将编码后的文件加入上传队列
        }
    }

    void StreamProcessor::cleanup()
    {
        for (auto &t : workers)
        {
            if (t.joinable()) // 如果线程可连接，则连接线程
                t.join();
        }
        clearTempFiles(); // 清理临时文件
    }

    void StreamProcessor::clearTempFiles()
    {
        while (true)
        {
            auto file = frameQueue->pop(); // 从帧队列中取出文件并删除
            if (file.empty())
                break;
            std::remove(file.c_str());
        }
        while (true)
        {
            auto file = uploadQueue.pop(); // 从上传队列中取出文件并删除
            if (file.empty())
                break;
            std::remove(file.c_str());
        }
    }
} // namespace VideoStreamer