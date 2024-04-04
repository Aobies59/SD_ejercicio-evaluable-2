#!/bin/bash
echo "# EXECUTING: $0"
echo -e

# TEST SERVICES WITH 1-SERVER-1-CLIENT
./test-1-server-1-client-init.sh &
wait $!
echo -e
./test-1-server-1-client-set_value.sh &
wait $!
echo -e
./test-1-server-1-client-exist.sh &
wait $!
echo -e
./test-1-server-1-client-modify_value.sh &
wait $!
echo -e
./test-1-server-1-client-get_value.sh &
wait $!
echo -e
./test-1-server-1-client-delete_key.sh &
wait $!