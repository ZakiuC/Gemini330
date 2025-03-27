cmake_minimum_required(VERSION 3.0)

# 设置项目名称
project(main)

# 设置 C++ 标准为 C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置阿里云OSS C++ SDK的路径（调整路径根据实际位置）
set(OSS_SDK_LIBRARY_PATH "/usr/local/lib/libalibabacloud-oss-cpp-sdk.a")

# 添加阿里云OSS C++ SDK的包含目录
include_directories(/usr/local/include/alibabacloud/oss)

# 修改SDL2查找方式
find_package(SDL2 REQUIRED)
if(TARGET SDL2::SDL2)
    message(STATUS "Found SDL2 CMake targets")
else()
    # 传统方式兼容
    message(STATUS "Using legacy SDL2 linking")
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}"
    )
endif()

# 设置库搜索路径为ARM64库路径
link_directories(
    ${OSS_SDK_LIB_DIR}
    /usr/lib/aarch64-linux-gnu
)

# 查找并配置OpenCV
find_package(OpenCV REQUIRED)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV library not found.")
endif()

# 查找依赖库 (OpenSSL 和 cURL)
find_package(OpenSSL REQUIRED)
if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL library not found.")
endif()

find_package(CURL REQUIRED)
if(NOT CURL_FOUND)
    message(FATAL_ERROR "cURL library not found.")
endif()


# 手动指定FFmpeg库路径
set(FFMPEG_ROOT "/usr/local/ffmpeg_build")  # 指向实际安装路径
set(FFMPEG_LIB_DIR "${FFMPEG_ROOT}/lib")
set(FFMPEG_INCLUDE_DIR "${FFMPEG_ROOT}/include")

message(STATUS "FFmpeg include dir: ${FFMPEG_INCLUDE_DIR}")
message(STATUS "FFmpeg lib dir: ${FFMPEG_LIB_DIR}")

# 更新包含目录
include_directories(
    ${FFMPEG_INCLUDE_DIR}  # 添加 FFmpeg 头文件路径
    # 其他包含目录...
)

# 查找FFmpeg库
find_library(AVFORMAT_LIB avformat PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
find_library(AVCODEC_LIB avcodec PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
find_library(AVUTIL_LIB avutil PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
find_library(SWSCALE_LIB swscale PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
find_library(SWRESAMPLE_LIB swresample PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
find_library(AVDEVICE_LIB avdevice PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)

# 检查所有库是否都已找到
if(NOT AVFORMAT_LIB OR NOT AVCODEC_LIB OR NOT AVUTIL_LIB OR NOT SWSCALE_LIB OR NOT SWRESAMPLE_LIB OR NOT AVDEVICE_LIB)
    message(FATAL_ERROR "Missing FFmpeg libraries. Found:
        avformat: ${AVFORMAT_LIB}
        avcodec: ${AVCODEC_LIB}
        avutil: ${AVUTIL_LIB}
        swscale: ${SWSCALE_LIB}
        swresample: ${SWRESAMPLE_LIB}
        avdevice: ${AVDEVICE_LIB}")
endif()

# 定义Orbbec SDK路径（相对于当前工作目录）
set(OB_SDK_PATH "/usr/local/OrbbecSDK")

# 包含Orbbec SDK、OpenCV以及FFmpeg的头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/demo/cpp  # 如果 window.hpp 位于 demo/cpp 中（如适用）
    ${OB_SDK_PATH}/include  # 添加 Orbbec SDK 的头文件路径
    ${OB_SDK_PATH}/include/libobsensor  # 显式添加 libobsensor 头文件路径
    ${OpenCV_INCLUDE_DIRS}
    "/usr/include/aarch64-linux-gnu"  # FFmpeg头文件路径

    # 添加 window.hpp 所在的目录
    ${OB_SDK_PATH}/examples/cpp  # window.hpp 所在的路径
    ${OB_SDK_PATH}/examples/c  # 如果你需要这个路径，也可以加上
    ${FFMPEG_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

# 添加所有需要的包含目录
include_directories(
    /usr/local/include  # 阿里云OSS C++ SDK的包含目录
)

# 设置库搜索路径为系统库路径，适应于ARM架构
set(CMAKE_LIBRARY_PATH /usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu ${CMAKE_LIBRARY_PATH})

# 添加源文件
set(SOURCES 
main.cpp
camera_capture.cpp
oss_uploader.cpp
stream_processor.cpp
video_encoder.cpp)  # 确保这里的路径和文件名正确

# 添加可执行文件
add_executable(main ${SOURCES})

# 链接阿里云OSS C++ SDK库、FFmpeg、SDL2库以及依赖的库
target_link_libraries(main
    ${OSS_SDK_LIBRARY_PATH}
    pthread
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    ${OpenCV_LIBS}
    ${OB_SDK_PATH}/lib/arm64/libOrbbecSDK.so
    ${OB_SDK_PATH}/lib/arm64/libdepthengine.so
    ${OB_SDK_PATH}/lib/arm64/libob_usb.so
    # FFmpeg库按正确顺序链接
    ${AVFORMAT_LIB}
    ${AVCODEC_LIB}
    ${AVUTIL_LIB}
    ${SWSCALE_LIB}
    ${SWRESAMPLE_LIB}
    ${AVDEVICE_LIB}
    SDL2::SDL2
    dl
    m
    z
    bz2
    x264
    vpx
    mp3lame
    freetype
    ${CMAKE_THREAD_LIBS_INIT}
)

# 如果使用 C++14，必须链接 stdc++fs
if(CMAKE_CXX_STANDARD EQUAL 14)
    target_link_libraries(main stdc++fs)
endif()

# 确保可以在 ARM 架构上进行编译，无需强制 x86_64 架构限制
message(STATUS "Building for ${CMAKE_SYSTEM_PROCESSOR} architecture.")