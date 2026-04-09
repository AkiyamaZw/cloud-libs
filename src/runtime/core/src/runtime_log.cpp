#include "core/runtime_log.h"
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/bundled/format.h>
#include <cstdarg>

namespace cloud
{
// 静态映射数组，直接将LogLevel映射到spdlog级别
static const spdlog::level::level_enum log_level_map[] = {
	spdlog::level::trace,	// LogLevel::Trace
	spdlog::level::debug,	// LogLevel::Debug
	spdlog::level::info,	// LogLevel::Info
	spdlog::level::warn,	// LogLevel::Warn
	spdlog::level::err,		// LogLevel::Error
	spdlog::level::critical // LogLevel::Critical
};

class AppLogger::Impl
{
  public:
	Impl()
	{
		// 初始化 spdlog
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

	std::shared_ptr<spdlog::logger> logger_;
	static constexpr uint8_t queue_size_{5};
};

AppLogger::AppLogger()
	: impl_(std::make_unique<Impl>())
{
}

AppLogger::~AppLogger() = default;

AppLogger *AppLogger::GetInstance()
{
	static AppLogger instance;
	return &instance;
}

void *AppLogger::Logger() { return GetInstance()->impl_->logger_.get(); }

void AppLogger::Log(LogLevel level, const char *fmt, ...)
{
	auto logger = GetInstance()->impl_->logger_;
	// 直接通过数组索引获取spdlog级别，无需调用to_spdlog_level
	auto spd_level = log_level_map[static_cast<int>(level)];

	if (logger->should_log(spd_level))
	{
		va_list args;
		va_start(args, fmt);

		// 计算所需的缓冲区大小
		int size = vsnprintf(nullptr, 0, fmt, args) + 1;
		std::vector<char> buffer(size);
		vsnprintf(buffer.data(), size, fmt, args);

		va_end(args);
		logger->log(spd_level, buffer.data());
	}
}
} // namespace cloud