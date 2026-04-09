#pragma once
#include <memory>
#include <cstdint>

#define NEO_XSTR(s) NEO_STR(s)
#define NEO_STR(s) #s

namespace cloud
{

// 封装日志级别
enum class LogLevel
{
	Trace,
	Debug,
	Info,
	Warn,
	Error,
	Critical
};

class AppLogger
{
  public:
	~AppLogger();
	static AppLogger *GetInstance();
	static void *Logger();
	static void Log(LogLevel level, const char *fmt, ...);

  protected:
	AppLogger();

  protected:
	// 使用前向声明和智能指针
	class Impl;
	std::unique_ptr<Impl> impl_;
	static constexpr uint8_t queue_size_{5};
};

// 新的宏定义，使用自定义级别
#define INFO(...) cloud::AppLogger::Log(cloud::LogLevel::Info, __VA_ARGS__)
#define DEBUG_INFO(...) cloud::AppLogger::Log(cloud::LogLevel::Debug, __VA_ARGS__)
#define WARN(...) cloud::AppLogger::Log(cloud::LogLevel::Warn, __VA_ARGS__)
#define FATAL(...) cloud::AppLogger::Log(cloud::LogLevel::Error, __VA_ARGS__)
#define TRACE(...) cloud::AppLogger::Log(cloud::LogLevel::Trace, __VA_ARGS__)

// 保持原有宏以兼容现有代码
#define __SYS_LOGGER cloud::AppLogger::Logger()
#define SYS_VERIFY(exp, ...)                                                                       \
	if (!(exp))                                                                                    \
	{                                                                                              \
		cloud::AppLogger::Log(cloud::LogLevel::Error, __VA_ARGS__);                                \
		assert(false);                                                                             \
	}

#define SYS_NOT_IMPL(...) SYS_VERIFY(false, __VA_ARGS__)
} // namespace cloud