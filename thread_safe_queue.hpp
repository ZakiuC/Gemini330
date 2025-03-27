#pragma once
#include <queue>
#include <mutex>
#include <memory>

namespace VideoStreamer
{
    // 模板类：线程安全的队列
    template <typename T>
    class ThreadSafeQueue
    {
    public:
        /**
         * 将一个值压入队列
         */
        void push(const T &value)
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(value);
        }

        /**
         * 从队列中弹出一个值
         */
        T pop()
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (queue.empty())
                return T();
            auto val = queue.front();
            queue.pop();
            return val;
        }

        /**
         * 获取队列的大小
         */
        size_t size() const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.size();
        }

    private:
        std::queue<T> queue;    // 存储队列元素
        mutable std::mutex mutex;   // 保护队列的线程安全
    };
} // namespace VideoStreamer