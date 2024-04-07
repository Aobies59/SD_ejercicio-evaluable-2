#!/bin/bash

# Run in case the server is not closing properly, it will kill any program called ./server in the background
# Change directory to root directory
if [ "$(basename "$(pwd)")" = "tests" ]; then
    cd ..
fi

server_pids=$(pgrep -x server)

if [ -n "$server_pids" ]; then
    # Kill each server process
    for pid in $server_pids; do
        kill "$pid"
    done
fi
