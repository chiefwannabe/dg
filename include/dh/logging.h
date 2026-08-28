#ifndef DH_LOGGING_H
#define DH_LOGGING_H

typedef enum {
    DH_LOG_LEVEL_DEBUG = 0,
    DH_LOG_LEVEL_INFO,
    DH_LOG_LEVEL_WARN,
    DH_LOG_LEVEL_ERROR
} DHLogLevel;

void dh_log_init(DHLogLevel min_level);
void dh_log_msg(DHLogLevel level, const char *fmt, ...);

#define dh_log_debug(...) dh_log_msg(DH_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define dh_log_info(...)  dh_log_msg(DH_LOG_LEVEL_INFO,  __VA_ARGS__)
#define dh_log_warn(...)  dh_log_msg(DH_LOG_LEVEL_WARN,  __VA_ARGS__)
#define dh_log_error(...) dh_log_msg(DH_LOG_LEVEL_ERROR, __VA_ARGS__)

#endif /* DH_LOGGING_H */
