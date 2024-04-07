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

# Start the clients
./cliente init &
./cliente set_value 1 "set_value" 2 21.1532024 -97122100278156.128392 &
./cliente exist 1 &
./cliente modify_value 1 "modify_value" 4 0.126404821 -17002781826.001874 -662.2 11991879.11 &
./cliente get_value 1 &
./cliente delete_key 1 &

# Wait last client to finish
wait $!

# Stop the server
kill $SERVER_PID

# See tuples file contents
echo "# TUPLES FILE:"
cat ./tuples.csv