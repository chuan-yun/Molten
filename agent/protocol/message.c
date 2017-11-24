#include "protocol.h"

/* unpck response , return body */
char *protocol_unpack_res(char *buf, int buf_size, int *body_size) {
    uint16_t byte_order16 = 0;
    uint32_t byte_order32 = 0;

    uint16_t code = ntohs(*(uint16_t *)buf); buf += sizeof(uint16_t);
    uint32_t size = ntohl(*(uint32_t *)buf); buf += sizeof(uint32_t);
    uint32_t validate = ntohl(*(uint32_t *)buf); buf += sizeof(uint32_t);
    
    char *body = smalloc(size);
    memcpy(body, buf, size);
    *body_size = size;
    return body;
}

/* pack req */
char* protocol_pack_req(int type, const char *buf, int size, int *buf_size) {
    char *new_buf = smalloc(PROTOCOL_HEADER_SIZE + size);    
    char *tmp_buf = new_buf;
    int validate = PROTOCOL_REQ_VALIDATE(type, size);

    uint16_t byte_order16 = 0;
    uint32_t byte_order32 = 0;
    
    byte_order16 = htons(type);
    memcpy(tmp_buf, &byte_order16, sizeof(uint16_t)); tmp_buf += sizeof(uint16_t);
    
    byte_order32 = htonl(size);
    memcpy(tmp_buf, &byte_order32, sizeof(uint32_t)); tmp_buf += sizeof(uint32_t);

    byte_order32 = htonl(validate);
    memcpy(tmp_buf, &byte_order32, sizeof(uint32_t)); tmp_buf += sizeof(uint32_t);

    memcpy(tmp_buf, buf, size); 
    *buf_size = PROTOCOL_HEADER_SIZE + size;
    return new_buf;
}
