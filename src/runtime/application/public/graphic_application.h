#pragma once

namespace cloud
{

struct AppParam
{
	int window_width;
	int window_height;
	const char *window_title;
};

void SetupApp(AppParam *param);
} // namespace cloud