#include "server.h"

// all config in server, string not use const, alloc from heap
m_server server;

/* write pid file */
void write_pid_file() {
    FILE *fp = fopen(server.pid_file, "w");
    if (fp) {
        fprintf(fp, "%d\n", server.pid);
        fclose(fp);
    }
}

/* daemonize */
void daemonize() {
    int fd;
    if (fork() != 0) exit(0); //parent exit
    setsid();
    if ((fd = open("/dev/null", O_RDWR, 0)) != 0) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) close(fd);
    }
}

/* signal server stop */
static void sig_stop_server(int sig) {
    char *msg;
    switch(sig) {
    case SIGTERM:
        msg = "Received SIGTERM to shutdown ... ";
        break;
    case SIGINT:
        msg = "Received SIGINIT to shutdown ... ";
        break;
    default:
        msg = "Received SIGNAL to shutdown ... ";
    }
    server.prepare_stop = 1; 
    server.el->stop = 1;
    AGENT_SLOG(SLOG_DEBUG, "[server]agent server prepare to stop");
}

/* dump server status */
static void dump_server_status(int sig) {
    sstring status =  server_status();
    sstring_print(status);
    sstring_free(status);
}

/* signal handler */
static void set_signal_handlers() {
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    struct sigaction act;
    
    //not mask other signal
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler =  sig_stop_server;
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);

    //we add debug handle for SIGUSR
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = dump_server_status;
    sigaction(SIGUSR1, &act, NULL);
    
    //we can add debug handle for SIGSEGV
}

/* init server */
static void server_init() {
    AGENT_SLOG(SLOG_DEBUG, "[server]agent server start prepare ...");

    if (server.daemon)  daemonize();
    server.pid_file = sstrdup(SERVER_DEFAULT_PID_FILE);
    server.pid = getpid();
    server.uptime = time(NULL);
    server.active_client_num = 0;
    server.total_client_num = 0;
    server.prepare_stop = 0;

    write_pid_file();
    
    server.worker_num = 20;
    server.wp = create_worker_pool(server.worker_num);

    /* read from config */
    //server.port = 12200;
    //server.log_file = NULL;

    /* todo lock pid file for one instance */

    set_signal_handlers();
    AGENT_SLOG(SLOG_DEBUG, "[server]agent server start ...");
}

/* server shutdwon */
static void server_shutdown() {
    destroy_worker_pool(server.wp);
    sfree(server.pid_file);
    AGENT_SLOG(SLOG_DEBUG, "[server]agent server stop");
}

static void server_config(int argc, char *argv[]) {
    //struct option long_options[] = {
    //    {"log_level",   required_argument, 0, 'l'},
    //    {"port",        required_argument, 0, 'p'}
    //};

    //int option_index = 0;
    //int c;
    //while((c == getopt_long(argc, argv, "l:p:" long_options, &option_index))) {
    //    switch(c) {
    //        case 'l':
    //            memcpy()
    //            break;
    //    }
    //}
     
}

// module one main thread to collect net io, other thread to execute  task
// module 
int main(int argc, char *argv[]) {
    /* server config */   

    slog_init(SLOG_STDOUT, "");
    server_init();
    
    int server_fd;
    if ((server_fd = create_tcp_listen_server("0.0.0.0", 12200, 256, AF_INET, NULL)) == A_FAIL) {
       AGENT_SLOG(SLOG_ERROR, "[server] create tcp listen socket error");
       exit(-1);
    }

    server.listen_fd = server_fd;
    event_loop *el = create_main_event_loop(1024);
    server.el = el;

    add_net_event(el, server_fd, NULL, accept_net_client, EVENT_READ);

    //uint64_t  period;
    // simple control info
    // one thread to collect info
    // 
    // time event need to process, 1 clear dead connection  2 heart beat  3 tick check

    execute_loop(server.el);

    server_shutdown();
    slog_destroy();
}
