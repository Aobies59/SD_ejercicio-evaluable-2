#!/bin/bash
echo "# EXECUTING: $0"

# Change directory to root directory
if [ "$(basename "$(pwd)")" = "tests" ]; then
    cd ..
fi

sh tests/kill_server.sh

# Compile project
make &> ./tests_output/temp_file.txt

# Delete the tuples file to ensure there are no leftover tuples
rm tuples.csv

# Define environment variables
export IP_TUPLAS="127.0.0.1"
export PORT_TUPLAS="8080"

# Start the server
./server &> ./tests_output/test-1-server-1-client-get-value-server-output.txt &

# Get its PID
SERVER_PID=$!

# Give the server time to start up
sleep 0.5

# Set a test tuple
./client set 1 "test" 2 1.4231 2231.0013 &> ./tests_output/test-1-server-1-client-get-value-client-output.txt &
# give the client some time to finish operating
sleep 0.5
./client get 1 &> ./tests_output/output_to_check.txt &

# Wait client to finish
wait $!

# Stop the server
./client exit &>>  ./tests_output/test-1-server-1-client-get-value-client-output.txt &

# Compare the exist output with the expected one (Key 1 exists)
echo "Tuple 1: value1 = 'test', N_Value2 = 2, V_Value2 = 1.42, 2231.00" > tests_output/temp_test_file.txt
if diff -q tests_output/output_to_check.txt tests_output/temp_test_file.txt; then
    echo "PASSED"
else
    echo "FAILED"
fi

rm tests_output/output_to_check.txt
rm tests_output/temp_test_file.txt