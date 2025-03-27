#pragma once
#include "config.hpp"
#include <memory>
#include <libobsensor/ObSensor.hpp>

namespace VideoStreamer
{
    /**
     * CameraCapture类用于从摄像头获取视频帧
     */
    class CameraCapture
    {
    public:
        /**
         * 接受一个配置对象来初始化
         */
        explicit CameraCapture(const AppConfig &cfg);
        ~CameraCapture();
        
        /**
         * 获取一帧图像，带有超时设置（默认为1000毫秒）
         */
        std::shared_ptr<ob::ColorFrame> getFrame(int timeoutMs = 1000);

    private:
        /**
         *  设置摄像头的数据流管道
         */
        void setupPipeline();
        
        /** 
         * 停止摄像头的流
         */
        void stop();

        // 配置文件对象，包含摄像头的相关配置信息
        AppConfig config;
        
        // 摄像头的数据流管道对象，用于管理摄像头的捕获流程
        ob::Pipeline pipeline;
    };
} // namespace VideoStreamer
