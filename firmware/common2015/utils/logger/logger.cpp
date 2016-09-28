#include "logger.hpp"

#include <cstdarg>
#include <algorithm>

#include <mbed.h>
#include <rtos.h>
#include <cstdio>

const char* LOG_LEVEL_STRING[] = {FOREACH_LEVEL(GENERATE_STRING)};
const char* LOG_FILE_NAMES[] = {"/local/LOG1.TXT", "/local/LOG2.TXT"};

const int maxLogSize = 250000;  // 250 KB each (500 KB total)

/* The initial logging level shows startup info along with any
 * warning messages [but hopefully there's none of those :) ].
 */
bool isLogging;  // = RJ_LOGGING_EN;

uint8_t rjLogLevel;

Mutex log_mutex;

LogHelper::LogHelper(uint8_t logLevel, const char* source, int line,
                     const char* func) {
    _logLevel = logLevel;
    _source = source;
    _line = line;
    _func = func;
}

LogHelper::~LogHelper() {
    log(_logLevel, _source, _line, _func, "%s", str().c_str());
}

/**
 * The system-wide logging interface function. All log messages go through
 * this.
 * @param logLevel The "importance level" of the called log message.
 * @param source   The source of the message.
 * @param format   The string format for displaying the log message.
 */
void log(uint8_t logLevel, const char* source, int line, const char* func,
         const char* format, ...) {
    if (isLogging && logLevel <= rjLogLevel) {
        log_mutex.lock();

        va_list args;
        static char newFormat[300];
        char time_buf[25];
        time_t sys_time = time(NULL);
        strftime(time_buf, 25, "%H:%M:%S", localtime(&sys_time));

        // If it's the first call to log(), find the right file to edit
        static uint8_t logNameInd = 255;
        if (logNameInd > 1) {
            logNameInd = findSmallestLogFile();
        }

        FILE* fp = fopen(LOG_FILE_NAMES[logNameInd], "a");

        if (fp) {
            fseek(fp, 0L, SEEK_END);

            if (ftell(fp) > maxLogSize) {
                // Deletes contents of the other file, and switches to it
                fclose(fp);

                logNameInd = (logNameInd + 1) % 2;

                fp = fopen(LOG_FILE_NAMES[logNameInd], "w");
            } else {
                // Returns to the correct location
                fseek(fp, 0L, SEEK_SET);
            }
        }

        snprintf(newFormat, sizeof(newFormat),
                 "%s [%s] [%s:%d] <%s>\r\n  %s\r\n\r\n", time_buf,
                 LOG_LEVEL_STRING[logLevel], source, line, func, format);

        va_start(args, format);

        fflush(stdout);
        vprintf(newFormat, args);
        fflush(stdout);

        if (fp) {
            vfprintf(fp, newFormat, args);
            fflush(fp);
            fclose(fp);
        }

        va_end(args);
        log_mutex.unlock();
    }
}

int logLvlChange(const std::string& s) {
    int n = 0;

    n += std::count(s.begin(), s.end(), '+');
    n -= std::count(s.begin(), s.end(), '-');

    return n;
}

// Finds the smallest log file in terms of file size
// Returns the index in reference to LOG_FILE_NAMES
uint8_t findSmallestLogFile() {
    FILE* fp = fopen(LOG_FILE_NAMES[0], "a");
    uint8_t index = 1;

    if (fp) {
        fseek(fp, 0L, SEEK_END);

        // The second file will not be used unless
        // the first reaches maxLogSize
        if (ftell(fp) <= maxLogSize) {
            index = 0;
        }
    }

    fclose(fp);

    return index;
}
