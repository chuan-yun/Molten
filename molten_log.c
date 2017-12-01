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

#include "molten_log.h" 

#define CLOSE_LOG_FD do {                   \
        close(log->fd);                     \
        log->fd = -1;                       \
}while(0)

static int mo_mkdir_recursive(const char *dir);
static void generate_log_path(mo_chain_log_t *log);

#ifdef HAS_CURL
/* {{{ trans log by http , current use curl not php_stream */
void send_data_by_http(char *post_uri, char *post_data)
{
    SLOG(SLOG_INFO, "[sink][http] http data sender, post_uri:%s", post_uri);
    if (post_uri != NULL && strlen(post_uri) > 5) {
        CURL *curl = curl_easy_init();
        if (curl) {
            CURLcode res;
            struct curl_slist *list = NULL;

            list = curl_slist_append(list, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_URL, post_uri);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

            res = curl_easy_perform(curl);
            //curl_easy_perform(curl);
            SLOG(SLOG_INFO, " curl request code:%d", res);
            curl_easy_cleanup(curl);
            curl_slist_free_all(list);
        } else {
            SLOG(SLOG_INFO, "[sink][http] init curl error");
        }
    }
}
/* }}} */
#endif

#ifdef HAS_KAFKA
/* {{{ kafka msg callback */
/*
static void dr_msg_cb(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque)
{
    if (rkmessage->err){
        MOLTEN_ERROR(" produce msg error:[%d]", rkmessage->err);
    } else {
        //todo record message
    }
}
*/
/* }}} */

/* {{{ trans log by kafka */
/*
static void trans_log_by_kafka(mo_chain_log_t *log, char *post_data)
{
    char errstr[512];
    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    if (rd_kafka_conf_set(conf, "bootstrap.servers", log->brokers, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        MOLTEN_ERROR(" new rd kafka conf error:[%s]", errstr);
        return;
    }

    rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb); 

    rd_kafka_t *rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));

    if (!rk) {
        MOLTEN_ERROR(" new rd kafka error:[%s]", errstr);
        return;
    }

    rd_kafka_topic_t *rkt = rd_kafka_topic_new(rk, log->topic, NULL);

    if (!rkt) {
        MOLTEN_ERROR(" new rd kafka topic error:[%s]", rd_kafka_err2str(rd_kafka_last_error()));
        rd_kafka_destroy(rk);
        return;
    }
    
    size_t len = strlen(post_data);
    if (rd_kafka_produce(rkt, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY, post_data, len, NULL, 0, NULL) == -1) {
        // todo record error
        if (rd_kafka_last_error() == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
            rd_kafka_poll(rk, 1000);
            // goto retry;
        }
    } else {
        // todo record sucess
    }
    
    rd_kafka_poll(rk, 0);

    rd_kafka_flush(rk, 100 );// wait for max 100 milliseconds

    // Destroy topic object
    rd_kafka_topic_destroy(rkt);

    // Destroy the producer instance
    rd_kafka_destroy(rk);
}
*/
/* }}} */
#endif

/* {{{ init syslog unix domain udp sink */
static void syslog_sink_init(mo_chain_log_t *log)
{
        SLOG(SLOG_INFO, "[sink][syslog] syslog data sender");
        if (log->unix_socket == NULL) {
            return;
        }

        struct sockaddr_un client;
        memset(&log->server, 0x00, sizeof(struct sockaddr_un));
        log->server.sun_family = AF_UNIX;
        strncpy(log->server.sun_path, log->unix_socket, sizeof(log->server.sun_path) - 1);

        log->sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (log->sfd == -1) {
            SLOG(SLOG_ERROR, " init syslog fd error: [%d] errstr[%s]", errno, strerror(errno));
            return;
        }

        memset(&client, 0x00, sizeof(struct sockaddr_un));
        client.sun_family = AF_UNIX;
        strncpy(client.sun_path, log->unix_socket, sizeof(client.sun_path) - 1);
        if (bind(log->sfd, (struct sockaddr *)&client, sizeof(struct sockaddr_un)) == -1) {
            SLOG(SLOG_ERROR, " bind syslog fd error: [%d] errstr[%s]", errno, strerror(errno));
            return;
        }
}
/* }}} */

/* {{{ syslog shutdown */
static void syslog_sink_shutdown(mo_chain_log_t *log) {
    if (log->sfd > 0) close(log->sfd);
}
/* }}} */

/* {{{ Log module ctor */
void mo_chain_log_ctor(mo_chain_log_t *log, char *host_name, char *log_path, long sink_type, long output_type, char *post_uri, char *syslog_unix_socket)
{
    log->path = log_path;
    log->tm_yday = -1;
    log->fd = -1;
    log->ino = 0;
    log->host_name = host_name;
    log->sink_type = sink_type;
    log->unix_socket = syslog_unix_socket;
    log->output_type = output_type;
    log->post_uri = post_uri;
    memset(log->real_path, 0x00, sizeof(log->real_path));
    log->format = LOG_FORMAT;
    log->buf = pemalloc(ALLOC_LOG_SIZE, 1);
    log->total_size = ALLOC_LOG_SIZE;
    log->alloc_size = 0;
    
    /* set support type */
    log->support_type = SINK_LOG | SINK_STD | SINK_SYSLOG ;
#ifdef HAS_CURL
    log->support_type |= SINK_HTTP;
    SLOG(SLOG_INFO, "[sink] has libcurl");
#endif

#ifdef HAS_KAFKA
    log->support_type |= SINK_KAFKA;
#endif
    
    /* todo for func cb, current use if else */
    if (log->sink_type == SINK_LOG) {
        generate_log_path(log);
    }

    if (log->sink_type == SINK_SYSLOG) {
        syslog_sink_init(log);
    }

    SLOG(SLOG_INFO, "[sink] current select sink_type:%d, input type%d", log->sink_type, sink_type);
}
/* }}} */

/* {{{ Log module dtor */
void mo_chain_log_dtor(mo_chain_log_t *log)
{

    SLOG(SLOG_INFO, "[sink] log module dtor");
    pefree(log->buf, 1);

    /* log fd close */
    if (log->sink_type == SINK_LOG) {
        if (log->fd != -1) {
            CLOSE_LOG_FD;
        }
    }

    /* unix fd close */
    if (log->sink_type == SINK_SYSLOG) {
        syslog_sink_shutdown(log);
    }
}
/* }}} */

/* {{{ Every init buffer */
void mo_chain_log_init(mo_chain_log_t *log)
{
    memset(log->buf, 0x00, log->total_size);
    log->alloc_size = 0; 
    MO_ALLOC_INIT_ZVAL(log->spans);
    array_init(log->spans);
}
/* }}} */

/* {{{ Add chain span */
void mo_chain_add_span(mo_chain_log_t *log, zval *span)
{
    /* Only for sampling record */
    /* can del after */
    if (log == NULL || log->spans == NULL) {
        SLOG(SLOG_ERROR, "[add span] log span is null");
        return;
    }
    add_next_index_zval(log->spans, span);
    MO_FREE_ALLOC_ZVAL(span);
}
/* }}} */

/* {{{ Add chain log */
void mo_chain_log_add(mo_chain_log_t *log, char *buf, size_t size)
{
    if (log->alloc_size + size >= (log->total_size + 1)) {
        int realloc_size = log->alloc_size + ((int)(size/ALLOC_LOG_SIZE) + 1) * ALLOC_LOG_SIZE;
        log->buf = perealloc(log->buf, realloc_size, 1);
        log->total_size = realloc_size;
    }
    strncpy(log->buf + log->alloc_size, buf, size);
    log->alloc_size  += size;
    
    /* add addtion break line only for std and file */
    if (log->sink_type <= SINK_STD) {
        strncpy(log->buf + log->alloc_size, "\n", 1);
        log->alloc_size++;
    }
}
/* }}} */

/* {{{ Recursive make dir */
static int mo_mkdir_recursive(const char *dir)
{
    if (access(dir, R_OK|W_OK) == 0) {
        return 0;
    }

    char tmp[PATH_MAX];
    strncpy(tmp, dir, PATH_MAX);
    int i, len = strlen(tmp);
    

    if (dir[len - 1] != '/')
    {
        strcat(tmp, "/");
    }

    len = strlen(tmp);

    for (i = 1; i < len; i++)
    {
        if (tmp[i] == '/')
        {
            tmp[i] = 0;
            if (access(tmp, R_OK) != 0)
            {
                if (mkdir(tmp, 0755) == -1)
                {
                    return -1;
                }
            }
            tmp[i] = '/';
        }
    }
    return 0;
}
/* }}} */

/* {{{ Generate log path */
static void generate_log_path(mo_chain_log_t *log)
{
    time_t raw_time;
    struct tm* time_info;
    char time_format[32];
    struct stat sb;
    memset(time_format, 0x00, 32);

    /* Check file exist */
    if ((log->fd != -1) && (access(log->real_path, F_OK) != 0)) {
        if (log->fd != -1) {
            CLOSE_LOG_FD;
        }
    }

    /* Check inode */
    if (log->fd != -1) {
        if (lstat(log->real_path, &sb) == -1) {
            if (log->fd != -1) {
                CLOSE_LOG_FD;
            }
        } else {
            if (sb.st_ino != log->ino) {
                if (log->fd != -1) {
                    CLOSE_LOG_FD;
                }
            }
        }
    }

    /* Check log change or not */
    time(&raw_time);
    time_info = localtime(&raw_time);
    if (time_info->tm_yday != log->tm_yday) {
        memset(log->real_path, 0x00, sizeof(log->real_path));
        strftime(time_format, 32, log->format, time_info);
        sprintf(log->real_path, "%s%s-%s.log", log->path, DEFAULT_PREFIX, time_format);
        log->tm_yday = time_info->tm_yday;
        if (log->fd != -1) {
            CLOSE_LOG_FD;
        }
    }
}
/* }}} */

/* {{{ flush log to fd */
static void inline flush_log_to_fd(mo_chain_log_t *log, char *bytes, int size)
{
    int written_bytes = 0;
    do {
        if ((written_bytes = write(log->fd, bytes, size) )== -1) {
            SLOG(SLOG_ERROR, "write log error[%d] errstr[%s]", errno, strerror(errno));
            return;
        }
        written_bytes += written_bytes;
    }while(written_bytes < size);
}
/* }}} */

/* {{{ flush log to syslog */
/* 
 * facility 20 (ocal use 4) and serverity 6 
 * see rfc3164 
 *
 */
static void inline flush_log_to_syslog(mo_chain_log_t *log, char *bytes, int size)
{
    if (log->sfd < 0) {
        return;
    }

    /* build syslog header */
    char str_time[64];
    char sys_log_header[256];
    int header_len;
    int send_len;
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL) {
        SLOG(SLOG_ERROR, "[sink][syslog] get local time error");
        return;
    }
    
    if (strftime(str_time, sizeof(str_time), "%b %d %H:%M:%S", tmp) == 0) {
        SLOG(SLOG_ERROR, "[sink][syslog] format strftime error");
        return;
    }

    memset(sys_log_header, 0x00, sizeof(sys_log_header));
    header_len = sprintf(sys_log_header, "<166> %s %s %s:", str_time, log->host_name, "molten");
    send_len = size + header_len; 

    /* here is two buffer, we need sendmsg */
    struct msghdr msg;
    struct iovec vec[2];
    msg.msg_name = &log->server;
    msg.msg_namelen = sizeof(struct sockaddr_un);
    msg.msg_iov = vec;
    msg.msg_iovlen = 2;
    
    vec[0].iov_base = sys_log_header;
    vec[0].iov_len = header_len;
    vec[1].iov_base = bytes;
    vec[1].iov_len = size;
    
    msg.msg_control = 0;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    if (sendmsg(log->sfd, &msg, 0) != send_len) {
        SLOG(SLOG_ERROR, "[sink][syslog] send msg error:[%d]", errno);
    }
}
/* }}} */

/* {{{ pt write info to log */
void mo_log_write(mo_chain_log_t *log, char *bytes, int size) 
{
    SLOG(SLOG_INFO, "[sink] mo log write sink_type [%d]", log->sink_type);
    switch (log->sink_type) {
        case SINK_STD:
            log->fd = 1;
            flush_log_to_fd(log, bytes, size);
            break;
        case SINK_LOG:

            if (mo_mkdir_recursive(log->path) == -1) {
                SLOG(SLOG_ERROR, "[sink][file] recursive make dir error [%s]", log->path);
                return;
            }

            generate_log_path(log);

            if (log->fd == -1) {
                log->fd = open(log->real_path, O_WRONLY|O_CREAT|O_APPEND, 0666);
                if (log->fd == -1) {
                    SLOG(SLOG_ERROR, "[sink][file] open log error[%d] errstr[%s]", errno, strerror(errno));
                    return;
                }
                struct stat sb;
                if (lstat(log->real_path, &sb) != -1) {
                    log->ino = sb.st_ino;
                } 
            }
            flush_log_to_fd(log, bytes, size);
            break;
        case SINK_SYSLOG:
            flush_log_to_syslog(log, bytes, size);
            break;
#ifdef HAS_CURL
        case SINK_HTTP:
            send_data_by_http(log->post_uri, bytes);
            break;
#endif

#ifdef HAS_KAFKA
#endif
        default:
            SLOG(SLOG_ERROR, "[sink] input error type [%d]", log->sink_type);
            break;
    }
}
/* }}} */

/* {{{ Flush log */
void mo_chain_log_flush(mo_chain_log_t *log)
{
    SLOG(SLOG_INFO, "[sink] mo log flush ");
    smart_string tmp = {0};

    /* Init json encode function */
    zval func;
    MO_ZVAL_STRING(&func, "json_encode", 1);
    
    if (log->output_type == SPANS_BREAK) {
        /* Encode one span one line , easy for debug */
        HashTable *ht = Z_ARRVAL_P(log->spans);
        zval *span;
#if PHP_VERSION_ID < 70000
        for(zend_hash_internal_pointer_reset(ht); 
                zend_hash_has_more_elements(ht) == SUCCESS;
                zend_hash_move_forward(ht)) {
            if (mo_zend_hash_get_current_data(ht, (void **)&span) == SUCCESS) {
                mo_php_json_encode(&tmp, span, 0);
                if (smart_string_str(tmp) != NULL) {
                    mo_chain_log_add(log, smart_string_str(tmp), smart_string_len(tmp));
                    smart_string_free(&tmp);
                } else {
                    goto end;
                }
            }
        }
#else
        ZEND_HASH_FOREACH_VAL(ht, span) {
            if (MO_Z_TYPE_P(span) == IS_ARRAY) {
                mo_php_json_encode(&tmp, span, 0);
                if (smart_string_str(tmp) != NULL) {
                    mo_chain_log_add(log, smart_string_str(tmp), smart_string_len(tmp));
                    smart_string_free(&tmp);
                } else {
                    goto end;
                }
            }
        } ZEND_HASH_FOREACH_END();
#endif
    } else if (log->output_type == SPANS_WRAP) {
        /* Load span from log */
        mo_php_json_encode(&tmp, log->spans, 0);
        if (smart_string_str(tmp) != NULL) {
            mo_chain_log_add(log, smart_string_str(tmp), smart_string_len(tmp));
            smart_string_free(&tmp);
        } else {
            goto end;
        }
    }
     
    SLOG(SLOG_INFO, "[sink] mo log flush detail size:%d", log->alloc_size);
    mo_log_write(log, log->buf, log->alloc_size);
    
end:
    mo_zval_dtor(&func);
    mo_zval_ptr_dtor(&log->spans);
    MO_FREE_ALLOC_ZVAL(log->spans);
    log->spans = NULL;
}
/* }}} */
