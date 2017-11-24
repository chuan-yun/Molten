#ifndef __MOLTEN_AGENT_SERVER_H
#define __MOLTEN_AGENT_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <smalloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
//#include <bits/sigaction.h> 
//#include <bits/sigset.h> 

#include "common.h"
#include "socket.h"
#include "event.h"
#include "event_loop.h"
#include "worker_pool.h"

#define SERVER_DEFAULT_PID_FILE "/var/run/molten_agent.pid"

#endif
