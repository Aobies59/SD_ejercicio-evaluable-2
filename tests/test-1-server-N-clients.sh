#!/bin/bash
echo "# EXECUTING: $0"

# Change directory to root directory
if [ "$(basename "$(pwd)")" = "tests" ]; then
    cd ..
fi

sh tests/kill_server.sh

# kill the server in case it is running in the background
sh ./tests/kill_server.sh

# Compile project
make &> ./tests_output/temp_file.txt

# Delete the tuples.csv file to make sure there is no data left (the server creates it again)
rm tuples.csv

# Define environment variables
export IP_TUPLAS="127.0.0.1"
export PORT_TUPLAS="8080"

# Start the server
./server &> ./tests_output/test-1-server-N-client-server-output.txt &

# Get its PID
SERVER_PID=$!

# Give the server time to start up
sleep 0.5

# Start the clients, we will give some time between each client, just enought to ensure the order is the intented one
./client set 1 "test" 2 1.4231 2231.0013 &> ./tests_output/test-1-server-N-client-set-value-1-output.txt &
sleep 0.1
./client exist 1 &> ./tests_output/test-1-server-N-client-exist-1-output.txt &
sleep 0.1
./client set 2 "test" 1 1.4231 &> ./tests_output/test-1-server-N-client-set-value-2-output.txt &
sleep 0.1
./client exist 2 &> ./tests_output/test-1-server-N-client-exist-2-output.txt &
sleep 0.1
./client delete 1 &> ./tests_output/test-1-server-N-client-delete-1-output.txt &
sleep 0.1
./client exist 1 &> ./tests_output/test-1-server-N-client-exist-1-2-output.txt &

# Wait client to finish
wait $!

# Stop the server
./client exit &>> ./tests_output/temp_file.txt

# Compare output files with their expected content

# Function to compare output file with expected content
compare_output() {
    output_file="$1"
    expected_content="$2"

    # Compare output file with expected content
    if ! diff -q "$output_file" <(echo "$expected_content") > /dev/null; then
        echo "FAILED"
        return 1
    fi
}

# Flag to indicate if any comparison failed
failed=0

# Compare each output file with its expected content
compare_output "./tests_output/test-1-server-N-client-set-value-1-output.txt" "Value set correctly" || failed=1
compare_output "./tests_output/test-1-server-N-client-exist-1-output.txt" "Key 1 exists" || failed=1
compare_output "./tests_output/test-1-server-N-client-set-value-2-output.txt" "Value set correctly" || failed=1
compare_output "./tests_output/test-1-server-N-client-exist-2-output.txt" "Key 2 exists" || failed=1
compare_output "./tests_output/test-1-server-N-client-delete-1-output.txt" "Key 1 deleted correctly" || failed=1
compare_output "./tests_output/test-1-server-N-client-exist-1-2-output.txt" "Key 1 doesn't exist" || failed=1

# If any comparison failed, echo "FAILED" and return
if [ "$failed" -eq 1 ]; then
    echo "FAILED"
    exit 1
fi

# If all comparisons passed, echo "PASSED"
echo "PASSED"
