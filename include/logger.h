//
// Created by sourcer on 04/07/16.
//

#ifndef MIKE_OS_LOGGER_H
#define MIKE_OS_LOGGER_H

#include "stdarg.h"

typedef enum log_type {LOG_INFO, LOG_WARNING,
                       LOG_ERROR, LOG_DEBUG} log_type_t;

void log_print(log_type_t log_type, char* format, ...);

#endif //MIKE_OS_LOGGER_H
