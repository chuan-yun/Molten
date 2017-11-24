#include "net_client.h"
/* reader and writer */
/* writer buf */
/* read buf */
rwer* new_rwer() {
    rwer * rw = smalloc(sizeof(rwer));
    rw->buf_size = CLIENT_BUF_SIZE;

    rw->read_pos = 0;
    rw->write_pos = 0;
    rw->buf = smalloc(CLIENT_BUF_SIZE);
    return rw;
}

/* reduce boundary */
void reduce_rwbuf(rwer *rw) {
    char *new_buf = smalloc(rw->buf_size - rw->write_pos);
    sfree(rw->buf);
    rw->buf_size = rw->buf_size - rw->write_pos;
    rw->read_pos = rw->read_pos - rw->write_pos;
    rw->write_pos = 0;
    rw->buf = new_buf;
}

void append_rw_reader(rwer *rw, const char *string , int size) {
    AGENT_SLOG(SLOG_DEBUG, "[client] append rwriter string %s, size:%d", string, size);
    if (size + rw->read_pos < rw->buf_size) {
        memcpy(rw->buf + rw->read_pos, string , size);  
        rw->read_pos += size;
    } else {
        /* todo more simple add */
        /* memory align */
        int new_size = rw->buf_size + (((rw->read_pos + size) - rw->buf_size)/CLIENT_BUF_SIZE + 1) * CLIENT_BUF_SIZE;

        rw->buf = srealloc(rw->buf, new_size);
        rw->buf_size = new_size;
        memcpy(rw->buf + rw->read_pos, string, size);
        rw->read_pos += size;
    }
}

/* rewind read write pos to start */
int rwer_rewind(rwer *rw) {
    if (rw->write_pos == rw->read_pos) {
        rw->write_pos = 0; 
        rw->read_pos = 0;
        bzero(rw->buf, rw->buf_size);
    }
}

/* free reader writer */
void free_rwer(rwer *rw) {
    sfree(rw->buf);
    sfree(rw);
}

/* for reader/writer append */
static void appender(char **buf, int *cap, int *len, const char *string , int size) {
    AGENT_SLOG(SLOG_DEBUG, "[client] append buf string %s, size:%d", string, size);
    if (size + *len < *cap)  {
        memcpy(*buf + *len, string ,size);
        *len += size;
    } else {
        int new_size = *cap + (((*len + size) - *cap)/CLIENT_BUF_SIZE + 1) * CLIENT_BUF_SIZE;

        *buf = (char *)srealloc(*buf, new_size);
        *cap = new_size;
        memcpy(*buf + *len, string, size);
        *len += size; 
    }
}

/* reader */
reader* new_reader() {
    reader *r = smalloc(sizeof(reader));
    r->cap = CLIENT_BUF_SIZE;
    r->read_pos = 0;
    r->len = 0;
    r->read_buf = smalloc(r->cap);
    return r;
}

/* read size */
int reader_read_size(reader *r) {
    return r->len - r->read_pos;
}

char *reader_read_start(reader *r) {
    return r->read_buf + r->read_pos;
}

/* reduce reader */
void reduce_reader(reader *r) {
    /* here memory is overlap , use memmove */
    if (r->rewind_count > 50) {
        memmove(r->read_buf, r->read_buf + r->read_pos, (r->len - r->read_pos));
        r->read_pos = 0;
        r->len = r->len - r->read_pos;
        r->rewind_count = 0;
    }
}

/* if read reach len we will rewind */
/* but if the read cannot , we should reduce */
/* if use ring buffer, it not simple dilatation */
void rewind_reader(reader *r) {
    if (r->read_pos == r->len) {
        if (r->read_pos == 0) {
            return;
        }
        r->read_pos = r->len = 0;
        memset(r->read_buf, 0x00, r->cap);
        r->rewind_count = 0;
    } else {
        r->rewind_count++;
        reduce_reader(r);
    }
}

/* offset reader pos */
/* return 0 success -1 fail */
int forward_reader_pos(reader *r, int offset) {
    if (r->read_pos + offset > r->len) {
        return -1;
    }
    r->read_pos += offset;
    rewind_reader(r);
    return 0;
}

/* appen size to reader */
void append_reader(reader *r, const char *string, int size) {
    appender(&r->read_buf, &r->cap, &r->len, string , size);
}

void free_reader(reader *r) {
    sfree(r->read_buf);
    sfree(r);
}

/* new writer */
writer* new_writer() {
    writer *w = smalloc(sizeof(writer));
    w->cap = CLIENT_BUF_SIZE;
    w->write_pos = 0;
    w->len = 0;
    w->write_buf = smalloc(w->cap);
    return w;
}

void free_writer(writer *w) {
    sfree(w->write_buf);
    sfree(w);
}

/* reduce writer */
void reduce_writer(writer *w) {
    /* here memory is overlap , use memmove */
    if (w->rewind_count > 50) {
        memmove(w->write_buf, w->write_buf + w->write_pos, (w->len - w->write_pos));
        w->write_pos = 0;
        w->len = w->len - w->write_pos;
        w->rewind_count = 0;
    }
}

/* rewind writer */
void rewind_writer(writer *w) {
    if (w->write_pos == w->len) {
        w->write_pos = w->len = 0;
        memset(w->write_buf, 0x00, w->cap);
        w->rewind_count = 0;
    } else {
        w->rewind_count++;
    }
    reduce_writer(w);
}

void append_writer(writer *w, const char *string, int size) {
    appender(&w->write_buf, &w->cap, &w->len, string , size);
}

/* writer */
/* leve triger no need to write until EWOULDBLOCK */
/* use in single thread, no condition race */
int writer_write(writer *w, int fd) {
    char *start_pos;
    int need_write, nwrite = 0;

    do {
        w->write_pos += nwrite; 
        start_pos = w->write_buf + w->write_pos;
        need_write = w->len - w->write_pos;
        if (need_write <= 0) {
            break;
        }
        nwrite = write(fd, start_pos, need_write);
    }while (nwrite >= 0);

    
    /* success and need_write == 0 */
    if (nwrite >= 0 && need_write == 0) {
        rewind_writer(w);
        return RW_WRITE_FINISH;
    } else if (nwrite =-1) {
        /* other error is not forbident */
        if (errno == EAGAIN || errno == EINTR) {
            return RW_WRITE_CONTINUE;
        } else {
            return RW_WRITE_ERROR;
        }
    }
}

/* net client */
net_client *new_client(int fd, char *client_ip) {
    net_client *nc = smalloc(sizeof(net_client));  
    nc->fd = fd;
    nc->client_ip = sstring_init(client_ip, strlen(client_ip));

    nc->create_time = time(NULL);
    nc->r = new_reader();
    nc->w = new_writer();
    return nc;
}

/* free client */
void free_client(net_client *nc) {
    free_reader(nc->r);
    free_writer(nc->w);
    sstring_free(nc->client_ip);
    sfree(nc);
}
