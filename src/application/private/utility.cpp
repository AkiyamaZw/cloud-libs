#include "utility.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace cloud
{
AppLogger::~AppLogger() {}

AppLogger::AppLogger()
{
    spdlog::init_thread_pool(queue_size_, 1);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S][%^%l%$]:%v");
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/app_log.txt",
        1024 * 1024 * 1,
        5,
        true); // max size 1MB, max files:5, rotate on open
#ifdef WIN32
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    std::vector<spdlog::sink_ptr> sinks{console_sink, rotating_sink, msvc_sink};
#else
    std::vector<spdlog::sink_ptr> sinks{console_sink, rotating_sink};
#endif
    logger_ = std::make_shared<spdlog::async_logger>("applog",
                                                     sinks.begin(),
                                                     sinks.end(),
                                                     spdlog::thread_pool(),
                                                     spdlog::async_overflow_policy::block);
    logger_->set_level(spdlog::level::trace);
    spdlog::register_logger(logger_);
    spdlog::set_default_logger(logger_);
}

AppLogger *AppLogger::GetInstance()
{
    static AppLogger logger;
    return &logger;
}
spdlog::logger *AppLogger::Logger() { return GetInstance()->logger_.get(); }
} // namespace cloud