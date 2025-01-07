#pragma once

#include<vector>
#include<atomic>

#include "macros.hpp"

namespace Common {
    // A single producer single consumer lock-free queue
    template<typename T>
    class LFQueue final {
        private:
            std::atomic<size_t> head{0};
            std::atomic<size_t> tail{0};
            std::atomic<size_t> num_elem{0};
            std::vector<T> queue_;
        
        public:
            LFQueue(const size_t capacity) : queue_(std::vector<T>(capacity, T())){}

            auto getNextWriteLocation() noexcept {
                return &queue_[tail];
            }

            auto getNextReadLocation() noexcept {
                return size() ? &queue_[head] : nullptr;
            }
            
            auto size() const noexcept {
                return num_elem.load();
            }

            // Does not check for overwrite
            // Could increase capacity --> perf decrease
            // Throw an error?
            auto updateNextToWrite() noexcept {
                tail = (tail+1)%queue_.size();
                num_elem++;
            }

            auto updateNextToRead() noexcept {
                head = (head+1)%queue_.size();
                ASSERT(num_elem > 0, "Read an invalid element: " + std::to_string(pthread_self()));
                num_elem--;
            }


            LFQueue() = delete;
            LFQueue(const LFQueue &) = delete;
            LFQueue(const LFQueue &&) = delete;
            LFQueue& operator=(const LFQueue &) = delete;
            LFQueue& operator=(const LFQueue &&) = delete;
    };
}