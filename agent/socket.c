#include <socket.h>

/* set fd reuse */
static int set_addr_reuse(int fd) {
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse) == -1)) {
        return A_FAIL;
    }
    return A_SUCCESS;
}

/* set tcp nodelay */
int set_nodelay(int fd) {
    int value = 1; 
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)) == -1)  {
        return A_FAIL; 
    }
    return A_SUCCESS;
}

/* set fd no block */
int set_noblock(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        return A_FAIL;
    }

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        return A_FAIL;
    }

    return A_SUCCESS;
}

/* select AF_INET or AF_INET6 to create tcp server */
int create_tcp_listen_server(char *addr, int port, int backlog, int famlily, char *err) {
    int fd;
    char p[6] = {0}; /* port string max 65535 */
    snprintf(p , 6, "%d", port);
         
    /* todo adapt ipv6 */  
    /* keep simple to validate event poll */
    struct addrinfo hints, *serv, *iterator; 
    memset(&hints, 0x00, sizeof(struct addrinfo));    
    hints.ai_family = famlily;
    hints.ai_socktype = SOCK_STREAM;

    /* man getaddrinfo */
    hints.ai_flags = AI_PASSIVE; 
   
    if (getaddrinfo(addr, p, &hints, &serv) == -1) {
        return A_FAIL;
    }
    
    for (iterator = serv; iterator != NULL; iterator = iterator->ai_next)  {
        if ((fd = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol)) == -1) continue;

        /* for socket reuse */
        if (bind(fd, iterator->ai_addr, iterator->ai_addrlen) == -1) {
            close(fd);
            goto err;
        }

        if (listen(fd, backlog) == -1) {
            close(fd);
            goto err;
        }
        set_addr_reuse(fd);
        goto finish;
    }
err:
    if (iterator == NULL) {
        close(fd);
        fd = A_FAIL;
    }
finish:
    freeaddrinfo(serv);
    return fd;
}

