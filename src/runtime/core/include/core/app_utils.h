#pragma once

namespace cloud
{
#define NEO_XSTR(s) NEO_STR(s)
#define NEO_STR(s) #s

namespace Utility
{
/**
 * @brief 初始化应用环境
 * @details 该函数负责初始化应用运行所需的环境，包括平台特定的设置
 */
void AppEnvInit();

} // namespace Utility
} // namespace cloud