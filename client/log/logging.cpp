#include "logging.h"
#include "../kv_client/kv_client.h"

void logging_string(const char* filepath, const char* str)
{
    /*open the log file*/
    FILE* fp = fopen(filepath, "a");
    fprintf(fp, "%s", str);
    fclose(fp);

    #if PRINTF_LOG_INFO
        fprintf(fp, "%s", str);
    #endif
}

void logging_event(const char* filepath, const char* event, timeval* start, timeval* end)
{
    /*open the log file*/
    FILE* fp = fopen(filepath, "a");   

    fprintf(fp, "%s:\n", event);
    fprintf(fp, "    start: %ld.%ld\n", start->tv_sec, start->tv_usec);
    fprintf(fp, "    end:   %ld.%ld\n", end->tv_sec, end->tv_usec);

    /*close the log file*/
    fclose(fp);

    #if PRINTF_LOG_INFO
        fprintf(stdout, "%s:\n", event);
        fprintf(stdout, "    start: %ld.%ld\n", start->tv_sec, start->tv_usec);
        fprintf(stdout, "    end:   %ld.%ld\n", end->tv_sec, end->tv_usec);
    #endif
}

