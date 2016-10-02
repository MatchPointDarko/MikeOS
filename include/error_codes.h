#ifndef MIKE_OS_ERROR_CODES_H
#define MIKE_OS_ERROR_CODES_H

typedef enum error_code
{
    SUCCESS,
    FAILURE,
    NO_SUCH_FILE,
    NO_SUCH_DIRECTORY,
    INVALID_ARGUMENT,
    NOT_ENOUGH_RAM
} error_code_t;

#endif //MIKE_OS_ERROR_CODES_H
