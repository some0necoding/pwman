#include "../headers/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#define BUFF_SIZE 64
#define TIME_STR_LEN 24                     // "Www Mmm dd hh:mm:ss yyyy" = 24 chars
#define FORMAT_STR_LEN 12

char *format_message(const char *f_message, va_list f_args_list);

int logger(const char *log_file_path, const char *tag, const char *f_message, ...)
{
    time_t local_time = time(NULL);                                                     // local time as time_t

    va_list f_args_list;

    size_t time_len = TIME_STR_LEN;
    //size_t final_buff_len = BUFF_SIZE;
    size_t format_str_len = FORMAT_STR_LEN;

    struct tm *local_time_struct = localtime(&local_time);                              // local time as struct tm (so that strftime() function can be used)

    char *format_str = (char *) malloc(format_str_len + 1);
    char *local_time_str = (char *) malloc(time_len + 1);                               // local time as string
    char *final_buff;
    char *message;                                                                      // message after being formatted

    int formatted_chars = 0;

    FILE *log_file = fopen(log_file_path, "a");

    strcpy(format_str, "%s [%s]: %s\n"); // 6

    va_start(f_args_list, f_message);
    message = format_message(f_message, f_args_list);

    strftime(local_time_str, time_len + 1, "%c", local_time_struct);

    /*TEST*/

    size_t tag_len = strlen(tag);
    size_t msg_len = strlen(message);
    size_t final_buff_len = 6 + time_len + tag_len + msg_len;

    final_buff = (char *) malloc(final_buff_len + 1);

    /*if ((formatted_chars = */snprintf(final_buff, final_buff_len, format_str, local_time_str, tag, message);/*) != final_buff_len) {
        perror("test: out of buffer");
        return -1;
    }*/

    /*while (formatted_chars > final_buff_len) {
        free(final_buff);
        final_buff_len += BUFF_SIZE;
        final_buff = (char *) malloc(final_buff_len + 1);
        formatted_chars = snprintf(final_buff, final_buff_len, format_str, local_time_str, tag, message);
    }*/

    final_buff_len = strlen(final_buff);
    fwrite(final_buff, 1, final_buff_len, log_file);
    fclose(log_file);
    return 0;
}

char *format_message(const char *f_message, va_list f_args_list)
{
    size_t final_buff_size = BUFF_SIZE;
    char *final_buff = (char *) malloc(final_buff_size);
    
    while (vsprintf(final_buff, f_message, f_args_list) < 0) {
        free(final_buff);
        final_buff_size += BUFF_SIZE;
        final_buff = (char *) malloc(final_buff_size);
    }

    return final_buff;
}