#pragma once

#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class ThreadSafeQueue {
private:
    unsigned short MAX_QUEUE_SIZE = 4;

    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    void push(const std::string& item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if(queue_.size() < MAX_QUEUE_SIZE || item == "QUIT"){
                queue_.push(item);
            }
        }
        cv_.notify_one();
    }

    size_t size() {
        return queue_.size();
    }

    std::string wait_and_pop() {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this] {
            return !queue_.empty();
        });

        std::string value = queue_.front();
        queue_.pop();
        return value;
    }

    bool try_pop(std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (queue_.empty())
            return false;

        value = queue_.front();
        queue_.pop();
        return true;
    }
};

ThreadSafeQueue requests;
ThreadSafeQueue responses;