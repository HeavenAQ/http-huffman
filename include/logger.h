#ifndef _LOGGER_H_
#define _LOGGER_H_
typedef struct Logger Logger;
struct Logger {
    void (*info_log)(const char *msg, const char *file, const int line);
    void (*warn_log)(const char *msg, const char *file, const int line);
    void (*error_log)(const char *msg, const char *file, const int line);
};
extern void init_logger(Logger **self);
#endif
