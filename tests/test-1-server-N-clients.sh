#!/bin/bash
echo "# EXECUTING: $0"

readonly DIR_BIN="bin"
readonly DIR_QUEUES="/dev/mqueue/"
readonly DIR_SERVER_QUEUE="server_queue"
readonly DIR_CLIENT_QUEUES="client_queue_*"

# Change directory to root directory
cd ../

# Compile project
make

# Delete queues
rm -f "${DIR_QUEUES}${DIR_SERVER_QUEUE}"
rm -f "${DIR_QUEUES}${DIR_CLIENT_QUEUES}"

# Start the server
$DIR_BIN/servidor &

# Get its PID
SERVER_PID=$!

# Give the server time to start up
sleep 1

# Start the clients
$DIR_BIN/cliente init &
$DIR_BIN/cliente set_value 1 "set_value" 2 21.1532024 -97122100278156.128392 &
$DIR_BIN/cliente exist 1 &
$DIR_BIN/cliente modify_value 1 "modify_value" 4 0.126404821 -17002781826.001874 -662.2 11991879.11 &
$DIR_BIN/cliente get_value 1 &
$DIR_BIN/cliente delete_key 1 &

# Wait last client to finish
wait $!

# Stop the server
kill $SERVER_PID

# See data file contents
echo "# DATA FILE:"
cat data/data.csv

# Delete queues
rm -f "${DIR_QUEUES}${DIR_SERVER_QUEUE}"
rm -f "${DIR_QUEUES}${DIR_CLIENT_QUEUES}"
