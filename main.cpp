#include "stream_processor.hpp"
#include <chrono>
#include <thread>
#include <iostream>

int main() {
    using namespace VideoStreamer;
    
    AppConfig config;
    config.targetFPS = 15;  // 设置目标帧率为15帧每秒
    config.uploadThreads = 4;   // 设置上传线程数为4
    
    try {
        StreamProcessor processor(config);
        processor.start();
        std::this_thread::sleep_for(std::chrono::seconds(60));
        processor.stop();
    }
    catch(const std::exception& e) {
        std::cerr << "[main] Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}