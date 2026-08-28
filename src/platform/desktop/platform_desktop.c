#include "dh/platform.h"

const char *dh_platform_get_name(void)
{
#if defined(__linux__)
    return "Linux Desktop";
#elif defined(_WIN32)
    return "Windows Desktop";
#elif defined(__APPLE__)
    return "macOS Desktop";
#else
    return "Desktop";
#endif
}
