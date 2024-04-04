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

# Start the client
$DIR_BIN/cliente get_value 1 &

# Wait client to finish
wait $!

# Stop the server
kill $SERVER_PID

# See data file contents
echo "# DATA FILE:"
cat data/data.csv

# Delete queues
rm -f "${DIR_QUEUES}${DIR_SERVER_QUEUE}"
rm -f "${DIR_QUEUES}${DIR_CLIENT_QUEUES}"