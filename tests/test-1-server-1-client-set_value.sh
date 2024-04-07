#!/bin/bash
echo "# EXECUTING: $0"

# Change directory to root directory
cd ../

# Compile project
make

# Start the server
./servidor &

# Get its PID
SERVER_PID=$!

# Give the server time to start up
sleep 1

# Start the client
./cliente set_value 1 "set_value" 2 21.1532024 -97122100278156.128392 &

# Wait client to finish
wait $!

# Stop the server
kill $SERVER_PID

# See tuples file contents
echo "# TUPLES FILE:"
cat ./tuples.csv