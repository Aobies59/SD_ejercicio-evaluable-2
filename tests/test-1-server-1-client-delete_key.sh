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

# Start the client
./cliente delete 1 &

# Wait client to finish
wait $!

# Stop the server
kill $SERVER_PID

# See tuples file contents
echo "# TUPLES FILE:"
cat ./tuples.csv