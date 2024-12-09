#!/vendor/bin/sh

# RIL Test Suite for Lumia 950
# Tests RIL functionality, modem communication, and SIM status

log_test() {
    echo "[RIL_TEST] $1"
    log -t ril_test "$1"
}

test_device_nodes() {
    log_test "Testing device nodes..."
    
    # Check RIL device nodes
    for dev in msril cellmux cellusermode cellnetmon qcril_radio; do
        if [ -e "/dev/$dev" ]; then
            log_test "✓ /dev/$dev exists"
        else
            log_test "✗ /dev/$dev missing"
            return 1
        fi
        
        # Check permissions
        PERMS=$(ls -l /dev/$dev | awk '{print $1}')
        if [ "$PERMS" = "crw-r--r--" ]; then
            log_test "✓ /dev/$dev has correct permissions"
        else
            log_test "✗ /dev/$dev has wrong permissions: $PERMS"
            return 1
        fi
    done
    
    return 0
}

test_kernel_modules() {
    log_test "Testing kernel modules..."
    
    # Check if modules are loaded
    for module in msril cellmux cellusermodeinterconnect cellnetmon; do
        if lsmod | grep -q $module; then
            log_test "✓ $module module loaded"
        else
            log_test "✗ $module module not loaded"
            return 1
        fi
    done
    
    return 0
}

test_qmi_services() {
    log_test "Testing QMI services..."
    
    # Check QMI service status
    if ps -A | grep -q qmuxd; then
        log_test "✓ QMUXD service running"
    else
        log_test "✗ QMUXD service not running"
        return 1
    fi
    
    # Test QMI communication
    if qmicli -d /dev/qcril_radio --uim-get-card-status --timeout=30; then
        log_test "✓ QMI communication successful"
    else
        log_test "✗ QMI communication failed"
        return 1
    fi
    
    return 0
}

test_sim_status() {
    log_test "Testing SIM card status..."
    
    # Get SIM status
    CARD_STATUS=$(qmicli -d /dev/qcril_radio --uim-get-card-status)
    if [ $? -ne 0 ]; then
        log_test "✗ Failed to get SIM status"
        return 1
    fi
    
    # Check if SIM is present
    if echo "$CARD_STATUS" | grep -q "Card state: 'present'"; then
        log_test "✓ SIM card present"
    else
        log_test "✗ SIM card not present"
        return 1
    fi
    
    # Check PIN status
    if echo "$CARD_STATUS" | grep -q "PIN1 state: 'enabled-verified'"; then
        log_test "✓ SIM PIN verified"
    else
        log_test "✗ SIM PIN not verified"
        return 1
    fi
    
    return 0
}

test_network_registration() {
    log_test "Testing network registration..."
    
    # Get network registration status
    NET_STATUS=$(qmicli -d /dev/qcril_radio --nas-get-serving-system)
    if [ $? -ne 0 ]; then
        log_test "✗ Failed to get network status"
        return 1
    fi
    
    # Check registration state
    if echo "$NET_STATUS" | grep -q "Registration state: 'registered'"; then
        log_test "✓ Network registered"
    else
        log_test "✗ Network not registered"
        return 1
    fi
    
    # Check signal strength
    SIGNAL=$(qmicli -d /dev/qcril_radio --nas-get-signal-strength)
    if [ $? -eq 0 ] && [ -n "$SIGNAL" ]; then
        log_test "✓ Signal strength available"
    else
        log_test "✗ Signal strength not available"
        return 1
    fi
    
    return 0
}

test_data_connection() {
    log_test "Testing data connection..."
    
    # Check data service state
    if qmicli -d /dev/qcril_radio --wds-get-packet-service-status | grep -q "Connection status: 'connected'"; then
        log_test "✓ Data connection active"
    else
        log_test "✗ Data connection inactive"
        return 1
    fi
    
    return 0
}

# Main test sequence
main() {
    log_test "Starting RIL test suite..."
    
    TESTS_PASSED=0
    TESTS_FAILED=0
    
    # Run all tests
    if test_device_nodes; then
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    if test_kernel_modules; then
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    if test_qmi_services; then
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    if test_sim_status; then
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    if test_network_registration; then
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    if test_data_connection; then
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    # Print summary
    log_test "Test Summary:"
    log_test "Tests Passed: $TESTS_PASSED"
    log_test "Tests Failed: $TESTS_FAILED"
    
    if [ $TESTS_FAILED -eq 0 ]; then
        log_test "All tests passed successfully!"
        exit 0
    else
        log_test "Some tests failed. Check logs for details."
        exit 1
    fi
}

# Run tests
main 