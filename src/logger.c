#include "../include/logger.h"
#include "../include/utils.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static void info_logger(const char *msg, const char *filename, const int line)
{
    time_t now;
    time(&now);
    fprintf(stdout, "%s [INFO] %s:%d %s\n", ctime(&now), filename, line, msg);
}

static void warn_logger(const char *msg, const char *filename, const int line)
{
    time_t now;
    time(&now);
    fprintf(stderr, "%s [WARN] %s:%d %s\n", ctime(&now), filename, line, msg);
}

static void error_logger(const char *msg, const char *filename, const int line)
{
    time_t now;
    time(&now);
    fprintf(stderr, "%s [ERROR] %s:%d %s\n", ctime(&now), filename, line, msg);
}

void init_logger(Logger **self)
{
    *self = (Logger *)must_calloc(1, sizeof(Logger));
    (*self)->info_log = &info_logger;
    (*self)->warn_log = &warn_logger;
    (*self)->error_log = &error_logger;
    (*self)->info_log("Logger initialized", __FILE__, __LINE__);
}
