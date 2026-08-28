#include "dh/logging.h"
#include <stdio.h>
#include <stdarg.h>

#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
#include <android/log.h>
#define LOG_TAG "DungeonHunter"
#endif

static DHLogLevel g_min_log_level = DH_LOG_LEVEL_DEBUG;

void dh_log_init(DHLogLevel min_level)
{
    g_min_log_level = min_level;
}

void dh_log_msg(DHLogLevel level, const char *fmt, ...)
{
    if (level < g_min_log_level) {
        return;
    }

    va_list args;
    va_start(args, fmt);

#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
    int android_priority = ANDROID_LOG_INFO;
    switch (level) {
        case DH_LOG_LEVEL_DEBUG: android_priority = ANDROID_LOG_DEBUG; break;
        case DH_LOG_LEVEL_INFO:  android_priority = ANDROID_LOG_INFO;  break;
        case DH_LOG_LEVEL_WARN:  android_priority = ANDROID_LOG_WARN;  break;
        case DH_LOG_LEVEL_ERROR: android_priority = ANDROID_LOG_ERROR; break;
    }
    __android_log_vprint(android_priority, LOG_TAG, fmt, args);
#else
    const char *prefix = "[INFO]";
    FILE *stream = stdout;

    switch (level) {
        case DH_LOG_LEVEL_DEBUG:
            prefix = "[DEBUG]";
            break;
        case DH_LOG_LEVEL_INFO:
            prefix = "[INFO]";
            break;
        case DH_LOG_LEVEL_WARN:
            prefix = "[WARN]";
            stream = stderr;
            break;
        case DH_LOG_LEVEL_ERROR:
            prefix = "[ERROR]";
            stream = stderr;
            break;
    }

    fprintf(stream, "%s ", prefix);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
    fflush(stream);
#endif

    va_end(args);
}
