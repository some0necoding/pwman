#include "../headers/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#define BUFF_SIZE 64

char *format_message(const char *f_message, va_list f_args_list);

void logger(const char *log_file_path, const char *tag, const char *f_message, ...)
{
    time_t local_time = time(NULL);                                                     // local time as time_t

    va_list f_args_list;

    size_t time_len = 24;                                                               // "Www Mmm dd hh:mm:ss yyyy" = 24 chars
    size_t final_buff_len = BUFF_SIZE;

    struct tm *local_time_struct = localtime(&local_time);                              // local time as struct tm (so that strftime() function can be used)

    char *format_str = (char *) malloc(15);
    char *local_time_str = (char *) malloc(time_len + 1);                               // local time as string
    char *final_buff = (char *) malloc(final_buff_len);
    char *message;                                                                      // message after being formatted

    FILE *log_file = fopen(log_file_path, "a");

    strcpy(format_str, "%s [%s]: %s\n");

    va_start(f_args_list, f_message);
    message = format_message(f_message, f_args_list);

    strftime(local_time_str, time_len+1, "%c", local_time_struct);

    while (sprintf(final_buff, format_str, local_time_str, tag, message) < 0) {
        final_buff_len += BUFF_SIZE;
        final_buff = (char *) realloc(final_buff, final_buff_len);
    }

    final_buff_len = strlen(final_buff);
    fwrite(final_buff, 1, final_buff_len, log_file);
    fclose(log_file);
}

char *format_message(const char *f_message, va_list f_args_list)
{
    size_t final_buff_size = BUFF_SIZE;
    char *final_buff = (char *) malloc(final_buff_size);
    
    while (vsprintf(final_buff, f_message, f_args_list) < 0) {
        final_buff_size += BUFF_SIZE;
        final_buff = (char *) realloc(final_buff, final_buff_size);
    }

    return final_buff;
}