#!/bin/bash

# RIL Test Runner for Lumia 950
# Runs all RIL tests and collects results

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() {
    echo -e "${2:-$NC}$1${NC}"
}

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    log "Please run as root" "$RED"
    exit 1
fi

# Create test directory
TEST_DIR="/data/local/tmp/ril_test"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR" || exit 1

# Compile kernel module test
log "Compiling RIL test module..." "$YELLOW"
make -C /vendor/lib/modules/$(uname -r)/build M=$(pwd) modules
if [ $? -ne 0 ]; then
    log "Failed to compile test module" "$RED"
    exit 1
fi

# Run shell tests
log "\nRunning shell tests..." "$YELLOW"
/vendor/bin/ril_test.sh
SHELL_TEST_RESULT=$?

# Load test module
log "\nLoading test module..." "$YELLOW"
insmod ril_module_test.ko
if [ $? -ne 0 ]; then
    log "Failed to load test module" "$RED"
    exit 1
fi

# Run module tests
log "\nRunning module tests..." "$YELLOW"
echo "sim" > /dev/riltest
echo "signal" > /dev/riltest
echo "system" > /dev/riltest

# Wait for tests to complete
sleep 2

# Check kernel log for test results
DMESG_OUTPUT=$(dmesg | grep "RIL_TEST" | tail -n 20)
MODULE_TEST_RESULT=0

if echo "$DMESG_OUTPUT" | grep -q "Failed"; then
    MODULE_TEST_RESULT=1
fi

# Unload test module
rmmod ril_module_test

# Print test summary
log "\nTest Summary:" "$YELLOW"
log "----------------------------------------"

if [ $SHELL_TEST_RESULT -eq 0 ]; then
    log "Shell Tests: PASSED" "$GREEN"
else
    log "Shell Tests: FAILED" "$RED"
fi

if [ $MODULE_TEST_RESULT -eq 0 ]; then
    log "Module Tests: PASSED" "$GREEN"
else
    log "Module Tests: FAILED" "$RED"
fi

log "----------------------------------------"

# Print detailed results
log "\nDetailed Test Results:" "$YELLOW"
log "----------------------------------------"
log "Shell Test Output:"
/vendor/bin/ril_test.sh -v

log "\nModule Test Output:"
echo "$DMESG_OUTPUT"
log "----------------------------------------"

# Clean up
rm -rf "$TEST_DIR"

# Exit with combined result
if [ $SHELL_TEST_RESULT -eq 0 ] && [ $MODULE_TEST_RESULT -eq 0 ]; then
    log "\nAll tests passed successfully!" "$GREEN"
    exit 0
else
    log "\nSome tests failed. Check detailed results above." "$RED"
    exit 1
fi 