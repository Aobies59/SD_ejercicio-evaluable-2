#!/bin/bash
echo "# EXECUTING: $0"

# Change directory to root directory
if [ "$(basename "$(pwd)")" = "tests" ]; then
    cd ..
fi

sh tests/kill_server.sh

# Compile project
make &> ./tests_output/temp_file.txt

# Delete the tuples.csv file to make sure there is no data left (the server creates it again)
rm tuples.csv

# Define environment variables
export IP_TUPLAS="127.0.0.1"
export PORT_TUPLAS="8080"

# Start the server
./server &> ./tests_output/test-1-server-1-client-delete-key-server-output.txt &

# Get its PID
SERVER_PID=$!

# Give the server time to start up
sleep 0.5

# Start the client
./client set 1 "test" 2 1.4231 2231.0013 &> ./tests_output/test-1-server-1-client-delete-key-client-output.txt &

# Give the client some time to finish
sleep 0.5
./client delete 1 &>> ./tests_output/test-1-server-1-client-delete-key-client-output.txt &

# Wait client to finish
wait $!

# Stop the server
./client exit &>>  ./tests_output/test-1-server-1-client-delete-key-client-output.txt &

# Check if tuples.csv is empty (if its wordcount is 0)
if [ $(wc -c < tuples.csv) -eq 0 ]; then
    echo "PASSED"
else
    echo "FAILED"
fi
rm tests_output/temp_file.txt