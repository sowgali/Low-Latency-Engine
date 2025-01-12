#pragma once

#include <thread>
#include <iostream>
#include <atomic>
#include <unistd.h>
#include <sys/syscall.h>

#include "macros.hpp"

namespace Common{

    inline auto setThreadCore(int core_id) noexcept{
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);

        return (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0);
    }

    template <typename T, typename... Args>
    inline auto setAndCreateThread(int core_id, const std::string &name, T &&func, Args &&...args) noexcept{
        auto t = new std::thread([&](){
            if (core_id >= 0 && !setThreadCore(core_id))
            {
                FATAL("Failed to set core affinity for " + name + " " + std::to_string(pthread_self()) + " to " + std::to_string(core_id));
            }

            std::cerr << "Set core affinity for " << name << " " << pthread_self() << " to " << core_id << std::endl;

            std::forward<T>(func)((std::forward<Args>(args))...);
        });

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);

        return t;
    }

}