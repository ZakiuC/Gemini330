#pragma once
#include "config.hpp"
#include "camera_capture.hpp"
#include "oss_uploader.hpp"
#include "video_encoder.hpp"
#include "thread_safe_queue.hpp"
#include <atomic>
#include <vector>
#include <memory>
#include <thread>

namespace VideoStreamer
{
    /**
     * StreamProcessor类，用于处理视频流的捕获、编码、上传等任务
     */
    class StreamProcessor
    {
    public:
        StreamProcessor(const AppConfig &cfg);

        /**
         * 启动视频流处理
         */
        void start();

        /**
         * 停止视频流处理
         */
        void stop();

    private:
        /**
         * 设置上传工作线程
         */
        void setupUploadWorkers();

        /**
         * 处理上传任务
         */
        void processUpload(OSSUploader &uploader);

        /**
         * 启动视频帧处理循环
         */
        void startProcessingLoop();

        /**
         * 处理视频帧
         */
        void processFrame(int &frameCounter);

        /**
         * 处理新的视频帧
         */
        void handleNewFrame(std::shared_ptr<ob::ColorFrame> frame, int &counter);

        /**
         * 保存临时帧到文件
         */
        std::string saveTempFrame(std::shared_ptr<ob::ColorFrame> frame);

        /**
         * 管理视频帧队列，确保队列大小不超过配置的最大值
         */
        void manageFrameQueue(const std::string &filename);

        /**
         * 检查是否需要批量编码
         */
        void checkBatchEncoding(int counter);

        /**
         * 执行批量编码处理
         */
        void processBatchEncoding();

        /**
         * 清理工作，停止线程等
         */
        void cleanup();

        /**
         * 清除临时文件
         */
        void clearTempFiles();

        // 配置参数
        AppConfig config;

        // 摄像头捕获对象
        CameraCapture camera;

        // 视频编码器对象
        VideoEncoder encoder;

        // 运行状态标志
        std::atomic<bool> running{true};

        // 存储视频帧的队列
        std::shared_ptr<ThreadSafeQueue<std::string>> frameQueue;

        // 存储待上传文件的队列
        ThreadSafeQueue<std::string> uploadQueue;

        // 工作线程池
        std::vector<std::thread> workers;
    };
} // namespace VideoStreamer