#pragma once
#include <fstream>

#include "macros.hpp"
#include "thread_utils.hpp"
#include "time_utils.hpp"
#include "spsc_lf_queue.hpp"

namespace Common {
    constexpr size_t LOG_QUEUE_SIZE = 1024 * 1024;

    enum class LogType : int8_t {
        CHAR = 0,
        INTEGER = 1,
        LONG_INTEGER = 2,
        LONG_LONG_INTEGER = 3,
        UNSIGNED_INTEGER = 4,
        UNSIGNED_LONG_INTEGER = 5,
        UNSIGNED_LONG_LONG_INTEGER = 6,
        FLOAT = 7,
        DOUBLE = 8
    };

    struct LogElement {
        LogType type_ = LogType::CHAR;

        union {
            char c;
            int i;
            long l;
            long long ll;
            unsigned u;
            unsigned long ul;
            unsigned long long ull;
            float f;
            double d;
        } u_;
    };

    class Logger final {
        
        private:
            const std::string fileName;
            std::ofstream fout;
            LFQueue<LogElement> queue_;
            std::thread* logger_thread_ = nullptr;

            std::atomic<bool> running_{true};
        
        public:

            auto flushQueue() noexcept {
                while(running_){
                    for(auto next = queue_.getNextReadLocation(); queue_.size() && next; next = queue_.getNextReadLocation()){
                        switch(next->type_){
                            case LogType::CHAR:
                                fout << next->u_.c;
                                break;
                            case LogType::DOUBLE:
                                fout << next->u_.d;
                                break;
                            case LogType::FLOAT:
                                fout << next->u_.f;
                                break;
                            case LogType::INTEGER:
                                fout << next->u_.i;
                                break;
                            case LogType::LONG_INTEGER:
                                fout << next->u_.l;
                                break;
                            case LogType::LONG_LONG_INTEGER:
                                fout << next->u_.ll;
                                break;
                            case LogType::UNSIGNED_INTEGER:
                                fout << next->u_.u;
                                break;
                            case LogType::UNSIGNED_LONG_INTEGER:
                                fout << next->u_.ul;
                                break;
                            case LogType::UNSIGNED_LONG_LONG_INTEGER:
                                fout << next->u_.ull;
                                break;
                        }
                        queue_.updateNextToRead();
                    }
                    fout.flush();

                    {
                        using namespace std::literals::chrono_literals;
                        std::this_thread::sleep_for(10ms);
                    }
                }
            }

            explicit Logger(const std::string& fname) : fileName(fname), queue_(LFQueue<LogElement>(LOG_QUEUE_SIZE)){
                fout.open(fileName);
                ASSERT(fout.is_open(), "Coud not open log file: " + fileName);
                logger_thread_ = setAndCreateThread(-1, "Common/Logger " + fileName, [this](){this->flushQueue();});
                ASSERT(logger_thread_ != nullptr, "Failed to start logger thread for " + fileName);
            }

            ~Logger(){
                std::string time_str;
                std::cerr << Common::getCurrentTimeStr(&time_str) << "Flushing and closing Logger for " << fileName << std::endl;

                while(queue_.size()){
                    using namespace std::literals::chrono_literals;
                    std::this_thread::sleep_for(1s);
                }
                running_ = false;
                fout.close();
                std::cerr << Common::getCurrentTimeStr(&time_str) << "Logger for " << fileName << " exiting." << std::endl;
            }

            auto pushValue(const LogElement& log_elem) noexcept {
                *(queue_.getNextWriteLocation()) = log_elem;
                queue_.updateNextToWrite();
            }

            auto pushValue(const char value) noexcept {
                pushValue(LogElement{LogType::CHAR, {.c = value}});
            }

            auto pushValue(const int value) noexcept {
                pushValue(LogElement{LogType::INTEGER, {.i = value}});
            }

            auto pushValue(const long value) noexcept {
                pushValue(LogElement{LogType::LONG_INTEGER, {.l = value}});
            }

            auto pushValue(const long long value) noexcept {
                pushValue(LogElement{LogType::LONG_LONG_INTEGER, {.ll = value}});
            }

            auto pushValue(const unsigned value) noexcept {
                pushValue(LogElement{LogType::UNSIGNED_INTEGER, {.u = value}});
            }

            auto pushValue(const unsigned long value) noexcept {
                pushValue(LogElement{LogType::UNSIGNED_LONG_INTEGER, {.ul = value}});
            }

            auto pushValue(const unsigned long long value) noexcept {
                pushValue(LogElement{LogType::UNSIGNED_LONG_LONG_INTEGER, {.ull = value}});
            }

            auto pushValue(const float value) noexcept {
                pushValue(LogElement{LogType::FLOAT, {.f = value}});
            }

            auto pushValue(const double value) noexcept {
                pushValue(LogElement{LogType::DOUBLE, {.d = value}});
            }

            auto pushValue(const char* value) noexcept {
                while(*value){
                    pushValue(*value);
                    value++;
                }
            }

            auto pushValue(const std::string& value) noexcept {
                pushValue(value.c_str());
            }

            template<typename T, typename... A>
            auto log(const char* s, const T& value, A... args) noexcept {
                while(*s){
                    if(*s == '%'){
                        if(*(s+1) == '%') [[unlikely]] {
                            s++;
                        } else {
                            pushValue(value);
                            log(s+1, args...);
                            return;
                        }
                    }
                    pushValue(*s++);
                }
                
                /* 
                    We are creating an override for const char*, 
                    it should return from prev ret
                    else we have extra params given to function.
                */
                FATAL("Extra arguments given to log() function.");
            }

            auto log(const char* s) noexcept {
                while(*s) {
                    if(*s == '%') {
                        if(*(s+1) == '%') [[unlikely]] {
                            s++;
                        } else {
                            FATAL("Fewer arguments given to log() function.");
                        }
                    }
                    pushValue(*s++);
                }
            }


    };


}