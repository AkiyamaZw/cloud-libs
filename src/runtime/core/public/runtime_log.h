#pragma once
#include <memory>
#include <spdlog/spdlog.h>
#include <cstdint>

#define NEO_XSTR(s) NEO_STR(s)
#define NEO_STR(s) #s

namespace cloud
{
namespace Utility
{
void AppEnvInit();
} // namespace Utility

class AppLogger
{
  public:
    ~AppLogger();
    static AppLogger *GetInstance();
    static spdlog::logger *Logger();

  protected:
    AppLogger();

  protected:
    std::shared_ptr<spdlog::logger> logger_;
    static constexpr uint8_t queue_size_{5};
};

#define __SYS_LOGGER cloud::AppLogger::Logger()

#define INFO(...) SPDLOG_LOGGER_CALL(__SYS_LOGGER, spdlog::level::info, __VA_ARGS__)

#define DEBUG(...) SPDLOG_LOGGER_CALL(__SYS_LOGGER, spdlog::level::debug, __VA_ARGS__)

#define WARN(...) SPDLOG_LOGGER_CALL(__SYS_LOGGER, spdlog::level::warn, __VA_ARGS__)

#define FATAL(...) SPDLOG_LOGGER_CALL(__SYS_LOGGER, spdlog::level::err, __VA_ARGS__)

#define TRACE(...) SPDLOG_LOGGER_CALL(__SYS_LOGGER, spdlog::level::trace, __VA_ARGS__)

#define SYS_VERIFY(exp, ...)                                                                       \
    if (!(exp))                                                                                    \
    {                                                                                              \
        SPDLOG_LOGGER_CALL(__SYS_LOGGER, spdlog::level::err, __VA_ARGS__);                         \
        assert(false);                                                                             \
    }

#define SYS_NOT_IMPL(...) SYS_VERIFY(false, __VA_ARGS__)
} // namespace cloud