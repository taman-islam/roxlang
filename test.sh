#!/bin/bash

# Build the compiler first
make

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "Running ROX Tests..."
echo "--------------------------------"

fail_count=0

run_test() {
    file=$1
    echo -n "Testing $file... "
    # Run the test and capture output/exit code
    output=$(./rox run "$file" 2>&1)
    exit_code=$?

    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}PASSED${NC}"
    else
        echo -e "${RED}FAILED${NC}"
        echo "$output"
        fail_count=$((fail_count + 1))
    fi
}

# Run verified algorithms
run_test "test/two_sum.rox"
run_test "test/valid_parentheses.rox"
run_test "test/binary_search.rox"
run_test "test/max_subarray.rox"
run_test "test/longest_substring.rox"

echo "--------------------------------"
if [ $fail_count -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}$fail_count tests failed.${NC}"
    exit 1
fi
