/**
 * Copyright 2017 chuan-yun silkcutKs <silkcutbeta@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "molten_slog.h"

static molten_slog slg;

void slog_init(int type, char *log_file) {
    if (type == SLOG_FILE) {
       slg.type = SLOG_FILE;
       slg.log_file = log_file;
       slg.fp = fopen(log_file, "a+");
    } else {
        slg.type = SLOG_STDOUT;
        slg.log_file = NULL;
        slg.fp = NULL;
    }
}

void slog_destroy() {
    if (slg.fp != NULL)  fclose(slg.fp);
}

void slog_record(int level, const char *file, int line, const char *fmt, ...) {
    va_list args;
    char format[256]    = {0};
    char time_buf[64]   = {0};
    char log_buf[512]   = {0};

    const char *color_format = "%s %s%-5s\x1b[0m  \x1b[90m%s:%d:\x1b[0m ";
    const char *normal_format = "%s %-5s %s:%d: ";
    
    time_t t = time(NULL); 
    struct tm *lt = localtime(&t);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", lt);

    if (slg.type == SLOG_FILE) {
        sprintf(log_buf, normal_format, time_buf, level_names[level], file, line);
    } else {
        sprintf(log_buf, color_format, time_buf, level_colors[level], level_names[level], file, line);
    }

    va_start(args, fmt);
    vsprintf(log_buf + strlen(log_buf), fmt, args);
    va_end(args);

    strcat(log_buf, "\n");
    if (slg.type == SLOG_FILE) {
        fprintf(slg.fp, log_buf);
    } else {
        fprintf(stdout, log_buf);
    }
}
