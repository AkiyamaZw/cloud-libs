#include "core/app_utils.h"
#include <cstdlib>

namespace cloud
{
namespace Utility
{
void AppEnvInit()
{
#if defined(__APPLE__)
	char const *vk_layer_path = NEO_XSTR(NEO_VK_LAYER_PATH);
	char const *vk_icd_filenames = NEO_XSTR(NEO_VK_ICD_FILENAMES);
	setenv("VK_LAYER_PATH", vk_layer_path, 1);
	setenv("VK_ICD_FILENAMES", vk_icd_filenames, 1);
#endif
}

} // namespace Utility
} // namespace cloud