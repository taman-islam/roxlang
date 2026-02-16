#!/bin/bash

# Parse arguments for --clean flag
for arg in "$@"
do
    if [ "$arg" == "--clean" ]; then
        echo "Cleaning build..."
        make clean
    fi
done

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

test_fail() {
    file=$1
    expected_error=$2
    echo -n "Testing expected fail $file... "
    output=$(./rox run "$file" 2>&1)
    exit_code=$?

    if [ $exit_code -ne 0 ]; then
        if [ -n "$expected_error" ]; then
            if echo "$output" | grep -q "$expected_error"; then
                echo -e "${GREEN}PASSED (Failed as expected with correct error)${NC}"
            else
                echo -e "${RED}FAILED (Failed but with wrong error)${NC}"
                echo "Expected error containing: $expected_error"
                echo "Actual output:"
                echo "$output"
                fail_count=$((fail_count + 1))
            fi
        else
            echo -e "${GREEN}PASSED (Failed as expected)${NC}"
        fi
    else
        echo -e "${RED}FAILED (Should have failed but passed)${NC}"
        echo "$output"
        fail_count=$((fail_count + 1))
    fi
}

# Run verified algorithms
run_test "test/binary_search.rox"
run_test "test/good_test.rox"
run_test "test/longest_substring.rox"
run_test "test/max_subarray.rox"
run_test "test/test_break.rox"
run_test "test/test_continue.rox"
run_test "test/test_cpp_collision.rox"
run_test "test/test_cpp_keywords.rox"
run_test "test/test_dict.rox"
run_test "test/test_format_not.rox"
run_test "test/test_format_out.rox"
run_test "test/test_format.rox"
run_test "test/test_functions_as_values.rox"
run_test "test/test_list_set.rox"
run_test "test/test_math.rox"
run_test "test/test_pop.rox"
run_test "test/test_regression.rox"
run_test "test/test_result_error.rox"
run_test "test/test_string.rox"
run_test "test/two_sum.rox"
run_test "test/valid_parentheses.rox"

# run tests that should fail
test_fail "test/test_roxv26_prefix.rox"
test_fail "test/test_string_fail.rox"
test_fail "test/test_flow_invalid_1.rox" "getValue(res) is unsafe"
test_fail "test/test_flow_invalid_2.rox" "getValue(res) is unsafe"
test_fail "test/test_dict_fail.rox" "Type Error: Dictionary value type mismatch"


echo "--------------------------------"
if [ $fail_count -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}$fail_count tests failed.${NC}"
    exit 1
fi
