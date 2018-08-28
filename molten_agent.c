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

#include "molten_agent.h"

static int quit = 0;

void mo_agent_monitor() {

}

void mo_send_to_agent() {
}

void mo_agent_collect_unix_sockect() {
}

void mo_agent_collect_mem() {

}

void mo_agent_signal(int signo) {
    switch(signo){
        case SIGHUP:
            quit = 1;
    }
}

void mo_agent_quit() {
    signal(SIGHUP, mo_agent_signal);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
}


void mo_agent_start() {
    pid_t pid = fork();
    if (pid < 0)  {
        /* fork child error */
        return;
    } else if (pid > 0) {
        /* parent process continue */
        return;
    }

    prctl(PR_SET_NAME, "molten_agent");
    pid_t ppid = getppid();

    /* init sink target */
    //mo_chain_log_ctor(&PTG(pcl), PTG(host_name), PTG(chain_log_path), PTG(sink_type), PTG(output_type), PTG(sink_http_uri), PTG(sink_syslog_unix_socket));
    
    /* init collector */
    /* monitor process */
    while(!quit) {
        sleep(1);
        printf("agent monitor parent id:%d\n", ppid);
    }
}
