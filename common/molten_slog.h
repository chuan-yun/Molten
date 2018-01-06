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
#ifndef MOLTEN_SLOG_H
#define MOLTEN_SLOG_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef MOLTEN_DEBUG
#define SLOG_INIT(type, log_file)               slog_init(type, log_file)
#define SLOG_DESTROY()                          slog_destroy()
#define SLOG(level, format, ...)                slog_record(level, __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define SLOG_INIT(type, log_file)
#define SLOG_DESTROY()
#define SLOG(level, format, ...)
#endif

/* log type */
#define SLOG_STDOUT     1
#define SLOG_FILE       2

/* log level */
#define SLOG_DEBUG 0
#define SLOG_INFO  1
#define SLOG_WARN  2
#define SLOG_ERROR 3
#define SLOG_FATAL 4

/* simple log just appen file */
typedef struct {
    char *log_file;
    FILE *fp;
    int type;
}molten_slog;

static const char *level_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
    "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

void slog_init(int type, char *log_file);
void slog_destroy();
void slog_record(int level, const char *file, int line, const char *fmt, ...);

#endif
