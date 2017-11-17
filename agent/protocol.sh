#!/bin/bash

# linux is little endian
# test echo protocol
printf "%b" '\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x66\x66'|nc 127.0.0.1 12200

# test time protocol
printf "%b" '\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x66\x66'|nc 127.0.0.1 12200

# test protocol truncte
printf "%b" '\x00\x00\x04\x00\x00\x00\x04\x00\x00\x00\x66\x66'|nc 127.0.0.1 12200

# get memory used
ps aux|grep molten_agent|grep -v grep|awk '{print $2}'|xargs kill -USR1
