#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdio.h>
#include <time.h>

void logging_string(const char* filepath, const char* str);
void logging_event(const char* filepath, const char* event, timeval* start, timeval* end);

#endif