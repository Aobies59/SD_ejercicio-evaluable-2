#!/bin/bash
echo "# EXECUTING: $0"

# Change directory to root directory
cd ../

# Compile project
make

# Define environment variables
export IP_TUPLAS="127.0.0.1"
export PORT_TUPLAS="8080"

# Start the server
./server &

# Get its PID
SERVER_PID=$!

# Give the server time to start up
sleep 1

# Start the clients
./client init &
./client set 1 "set_value" 2 21.152 -97121.182 &
./client exist 1 &
./client modify 1 "modify_value" 4 0.12401 -12.1209 9991.2 28271.001 &
./client get 1 &
./cliente delete 1 &

# Wait last client to finish
wait $!

# Stop the server
kill $SERVER_PID

# See tuples file contents
echo "# TUPLES FILE:"
cat ./tuples.csv