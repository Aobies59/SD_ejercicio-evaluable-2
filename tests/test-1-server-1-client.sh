#!/bin/bash

# Change directory to root directory
if [ "$(basename "$(pwd)")" = "tests" ]; then
    cd ..
fi

sh tests/kill_server.sh

# kill the server in case it is running in the background
sh ./tests/kill_server.sh

# TEST SERVICES WITH 1-SERVER-1-CLIENT
sh ./tests/test-1-server-1-client-init.sh &
wait $!
echo -e
sh ./tests/test-1-server-1-client-set_value.sh &
wait $!
echo -e
sh ./tests/test-1-server-1-client-exist.sh &
wait $!
echo -e
sh ./tests/test-1-server-1-client-modify_value.sh &
wait $!
echo -e
sh ./tests/test-1-server-1-client-get_value.sh &
wait $!
echo -e
sh ./tests/test-1-server-1-client-delete_key.sh &
wait $!