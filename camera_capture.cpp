#include "camera_capture.hpp"

namespace VideoStreamer
{
    CameraCapture::CameraCapture(const AppConfig &cfg) : config(cfg)
    {
        // 初始化摄像头的捕获管道
        setupPipeline();
    }

    CameraCapture::~CameraCapture()
    {
        stop();
    }

    // 获取一帧图像，timeoutMs为超时时间（单位毫秒），默认1000ms
    std::shared_ptr<ob::ColorFrame> CameraCapture::getFrame(int timeoutMs)
    {
        auto frameSet = pipeline.waitForFrames(timeoutMs);
        
        return frameSet && frameSet->colorFrame() ? frameSet->colorFrame() : nullptr;
    }

    /**
     * 设置摄像头的数据流管道，包括流的配置
     */
    void CameraCapture::setupPipeline()
    {
        auto profiles = pipeline.getStreamProfileList(OB_SENSOR_COLOR);
        
        auto profile = profiles->getVideoStreamProfile(
            config.targetWidth,
            config.targetHeight,
            config.colorFormat,
            config.targetFPS);

        auto cfg = std::make_shared<ob::Config>();
        cfg->enableStream(profile);
        
        // 启动数据流管道
        pipeline.start(cfg);
    }

    /** 
     * 停止摄像头的数据流
     */ 
    void CameraCapture::stop()
    {
        pipeline.stop();  // 停止流
    }
} // namespace VideoStreamer
