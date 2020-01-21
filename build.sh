#!/bin/sh
gcc hello.c -o hello $(pkg-config --cflags --libs libjerry-core libjerry-ext libjerry-port-default libjerry-libm)

gcc hello2.c -o hello2 $(pkg-config --cflags --libs libjerry-core libjerry-ext libjerry-port-default libjerry-libm)
