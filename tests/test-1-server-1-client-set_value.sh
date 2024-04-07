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
./server &> ./tests_output/test-1-server-1-client-set-value-server-output.txt &

# Get its PID
SERVER_PID=$!

# Give the server time to start up
sleep 0.5

# Start the client
./client set 1 "test" 2 1.4231 2231.0013 &>> ./tests_output/test-1-server-1-client-set-value-client-output.txt &

# Wait client to finish
wait $!

# Stop the server
./client exit &>>  ./tests_output/test-1-server-1-client-set-value-client-output.txt &

# Compare tuples.csv with the expected content
echo "1,2,test,1.423100,2231.001300" > tests_output/temp_test_file.txt
if diff -q tuples.csv tests_output/temp_test_file.txt; then
    echo "PASSED"
else
    echo "FAILED"
fi

rm tests_output/temp_file.txt
rm tests_output/temp_test_file.txt