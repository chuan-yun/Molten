#ifndef __MOLTEN_AGENT_PROTOCOL_PROTOCOL_H
#define __MOLTEN_AGENT_PROTOCOL_PROTOCOL_H

#include <netinet/in.h>

#include "common.h"
#include "net_client.h"

/* for request and response */
#define PROTOCOL_HEADER_SIZE    (sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t))

/* validate */
#define PROTOCOL_REQ_VALIDATE(type, size)    (type *2 + size)

/* protocol process */
#define PROTOCOL_ECHO           0
#define PROTOCOL_TIME           1
#define PROTOCOL_HEARTBEAT      2
#define PROTOCOL_STATUS         3

/* return code */
#define PROTOCOL_ERROR_MSG                  -1
#define PROTOCOL_READ_CONTINUE              0
#define PROTOCOL_NEED_REPLY                 1

/* response code */
#define RESPONSE_SUCESS         0
#define RESPONSE_FAIL           1

/* request */
typedef struct {
    /* header */
    uint16_t type; 
    uint32_t size;
    uint32_t validate;

    /* body */
    char *body;
}pro_req;


/* response */
typedef struct {

    /* header */
    uint16_t code;
    uint32_t size;
    uint32_t validate;  /* not use */

    /* body */
    char *body;
}pro_res;

typedef void protocol_process(net_client *nc, const char *body, int body_size);

/* for different protocol */
typedef struct {
    char *name;
    uint16_t sequence;
    protocol_process *process;
    uint8_t need_reply:1;           /* need reply */
}cmd;

char *protocol_unpack_res(char *buf, int buf_size, int *body_size);
char* protocol_pack_req(int type, const char *buf, int size, int *buf_size);
int protocol_analyze_req(net_client *nc);
#endif
