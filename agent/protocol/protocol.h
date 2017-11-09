#ifndef __MOLTEN_AGENT_PROTOCOL_PROTOCOL_H
#define __MOLTEN_AGENT_PROTOCOL_PROTOCOL_H

/* request */
typedef struct {

    /* header */
    uint16_t type; 
    uint32_t size;

    /* body */
    char *body;
}protocol_req;


/* response */
typedef struct {
    uint16_t code;
    uint32_t size;

    /* body */
    char *body;
}protocol_res;

void protocol_analyze(rwer *rw) {

}

#endif
