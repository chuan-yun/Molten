#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "protocol.h"
#include "smalloc.h"

void write_print(int fd, char *buf, int size) {
    int echo_size;
    char *echo = protocol_pack_req(PROTOCOL_ECHO, buf, size, &echo_size);
    int ret = write(fd, echo, echo_size);
    if (ret != echo_size) {
        printf("write error\n");
        return;
    }
    
    char read_buf[2056];
    int res = read(fd, read_buf, 2056);
    if (ret <= 0 ) {
        printf("read buf error\n");
        return;
    }
    
    int body_size;
    char *body = protocol_unpack_res(read_buf, res, &body_size);
    write(1, body, body_size);
    printf("\n");
    sfree(body);
}

int main(int argc, char **argv) {

    // first arg is  host, second is port
    char *host = argv[1];
    char *port = argv[2];
    int res;

    int nport = atoi(port);
    int server_fd;
    
    struct sockaddr_in      addr;
    addr.sin_family         = AF_INET;
    addr.sin_port           = htons(nport);
    addr.sin_addr.s_addr     = inet_addr(host);

    int cs = socket(AF_INET, SOCK_STREAM, 0);
    if ((res = connect(cs, (struct sockaddr*)&addr, sizeof(addr))) == -1)  {
        printf("error\n");
    }
    char *test = "echo";
    char *large = "asdfasfkjasdklfjsaklfjlkasdjffffffjfals;djfk;lasjfklasjlkfjaslkjflaskjflksa;jf;lkasdfja;sljfkl;asjfkl;sajfklasasdlfkj \
        aslfsadfasflk;jl;kjf;lasjfl;asjf;lksajf;klsajf;lkaj;lfkajl;kfdjaskl;jf;l";
    for (;;) {
        write_print(cs, large, strlen(large));
        write_print(cs, test, strlen(test));
        sleep(10);
    }
}
