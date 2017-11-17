#ifndef __MOLTEN_AGENT_CLIENT_H
#define __MOLTEN_AGENT_CLIENT_H

#include "common.h"
#include "errno.h"

#define CLIENT_BUF_SIZE    32

typedef struct{
    int read_pos;       /* read pos */
    int len;            /* real size */
    int cap;            /* capacity */
    int rewind_count;    /* rewind time reach will reduce */
    char *read_buf;     /* true buffer */
}reader;

typedef struct {
    int write_pos;       /* read pos */
    int len;            /* real size */
    int cap;            /* capacity */
    int rewind_count;   /* rewind time reach will reduce */
    char *write_buf;
}writer;

/* for reader and writer */
/* noneed to use ring buffer, the size we do not know */
typedef struct {
    int buf_size;
    int read_pos;
    int write_pos;
    char *buf;
}rwer;

/* for net client */
typedef struct {
    int fd;
    time_t create_time;
    sstring client_ip;
    time_t last_read;
    reader *r;
    writer *w;
}net_client;

/* reader and writer */
/* writer buf */
/* read buf */
rwer* new_rwer();

/* reduce boundary */
void reduce_rwbuf(rwer *rw);

void append_rw_reader(rwer *rw, const char *string , int size);

/* rewind read write pos to start */
int rwer_rewind(rwer *rw);

/* free reader writer */
void free_rwer(rwer *rw);

/* reader */
reader* new_reader();

/* read size */
int reader_read_size(reader *r);

char *reader_read_start(reader *r);

/* reduce reader */
void reduce_reader(reader *r);

/* if read reach len we will rewind */
/* but if the read cannot , we should reduce */
/* if use ring buffer, it not simple dilatation */
void rewind_reader(reader *r);

/* offset reader pos */
/* return 0 success -1 fail */
int forward_reader_pos(reader *r, int offset);

/* appen size to reader */
void append_reader(reader *r, const char *string, int size);
void free_reader(reader *r);

/* new writer */
writer* new_writer();

void free_writer(writer *w);

/* reduce writer */
void reduce_writer(writer *w);

/* rewind writer */
void rewind_writer(writer *w);

void append_writer(writer *w, const char *string, int size);

/* writer */
/* leve triger no need to write until EWOULDBLOCK */
/* use in single thread, no condition race */
int writer_write(writer *w, int fd);

// for echo server
void reader_to_writer(reader *r, writer *w);

/* net client */
net_client *new_client(int fd, char *client_ip);

/* free client */
void free_client(net_client *nc);
#endif
