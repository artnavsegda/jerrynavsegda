#!/bin/sh
gcc hello.c -o hello $(pkg-config --cflags --libs libjerry-core libjerry-ext libjerry-port-default libjerry-libm)

gcc api-example-1.c -o api-example-1 $(pkg-config --cflags --libs libjerry-core libjerry-port-default libjerry-libm)

