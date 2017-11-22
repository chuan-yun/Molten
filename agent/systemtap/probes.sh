#!/bin/bash
dtrace -C -G -s systemtap/probes.d -o systemtap/probes.o
dtrace -C -h -s systemtap/probes.d -o systemtap/probes.h
