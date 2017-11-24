#include "protocol.h"

/* add reply */
static void add_reply(net_client *nc, int success, const char *buf, uint32_t size) {
    char header_buf[PROTOCOL_HEADER_SIZE];
    char *tmp_buffer = header_buf;
    uint16_t result =  success ? RESPONSE_SUCESS : RESPONSE_FAIL;

    uint16_t byte_order16 = 0;
    uint32_t byte_order32 = 0;

    byte_order16 = htons(result);
    memcpy(tmp_buffer, &byte_order16, sizeof(uint16_t)); tmp_buffer += sizeof(uint16_t);

    byte_order32 = htonl(size);
    memcpy(tmp_buffer, &byte_order32, sizeof(uint32_t)); tmp_buffer += sizeof(uint32_t);
    memcpy(tmp_buffer, &byte_order32, sizeof(uint32_t));

    append_writer(nc->w, header_buf, PROTOCOL_HEADER_SIZE);
    append_writer(nc->w, buf, size);
}

/* echo server */
static void process_echo(net_client *nc, const char *body, int body_size) {
    AGENT_SLOG(SLOG_DEBUG, "[echo protocol] rec body :[%s]", body);
    AGENT_SLOG(SLOG_DEBUG, "[echo protocol] read buf :[%s], read pos:[%d], write pos:[%d], body_size:[%d]", nc->r->read_buf, nc->r->read_pos, nc->r->len, body_size);
    add_reply(nc, 1, body, body_size);
}

/* time server */
static void process_time(net_client *nc, const char *body, int body_size) {
    add_reply(nc, 1, "13.14", sizeof("13.14") - 1);
}

/* dump server status */
static void process_server_status(net_client *nc, const char *body, int body_size) {
    sstring status = server_status();  
    add_reply(nc, 1, status.s, sstring_length(status));
    sstring_free(status);
}

static cmd cmd_list[] = {
    {"echo",         PROTOCOL_ECHO,         process_echo,               1},
    {"time",         PROTOCOL_TIME,         process_time,               1},
    {"heartbeat",    PROTOCOL_TIME,         process_time,               1},
    {"status",       PROTOCOL_STATUS,       process_server_status,      1}
};

#define CMD_LIST_SIZE                       (sizeof(cmd_list)/sizeof(cmd))

/* analyze protocol */
int protocol_analyze_req(net_client *nc) {
    int can_read_size = reader_read_size(nc->r);
    if (can_read_size >= PROTOCOL_HEADER_SIZE) {

        char* read_pos = reader_read_start(nc->r);

        /* same host no need le/be problem */
        /* but feature will connect with other system adjust to net byte order */
        uint16_t type = ntohs(*(uint16_t *)read_pos); read_pos += sizeof(uint16_t);
        uint32_t size = ntohl(*(uint32_t *)read_pos); read_pos += sizeof(uint32_t);
        uint32_t validate = ntohl(*(uint32_t *)read_pos); read_pos += sizeof(uint32_t);

        /* validate */
        if (validate != (PROTOCOL_REQ_VALIDATE(type, size)))  {
            return PROTOCOL_ERROR_MSG;
        }
        
        /* read from buffer */
        int total_size = PROTOCOL_HEADER_SIZE + size;
        if (can_read_size >= total_size) {

            /* can not changed */
            const char* body = reader_read_start(nc->r) + PROTOCOL_HEADER_SIZE;

            /* process */
            if (type <= CMD_LIST_SIZE) {
                cmd_list[type].process(nc, body, size);
            } else {
                return PROTOCOL_ERROR_MSG;
            }
            
            /* forward_reader_pos */
            forward_reader_pos(nc->r, total_size);

            if (cmd_list[type].need_reply) {
                return PROTOCOL_NEED_REPLY;
            } else {
                return PROTOCOL_READ_CONTINUE;
            }
        } else {
            return PROTOCOL_READ_CONTINUE;
        }
    } else {
        return PROTOCOL_READ_CONTINUE;
    }
}
