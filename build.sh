#!/bin/sh
gcc hello.c -o hello $(pkg-config --cflags --libs libjerry-core libjerry-ext libjerry-port-default libjerry-libm)
