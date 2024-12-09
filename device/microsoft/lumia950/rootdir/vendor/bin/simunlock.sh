#!/vendor/bin/sh

# SimUnlock script for Lumia 950
# This needs to run with vendor privileges

# Wait for modem to be ready
while [ ! -e /dev/qcril_radio ]; do
    sleep 1
done

# Load necessary drivers
insmod /vendor/lib64/msril.sys
insmod /vendor/lib64/cellmux.sys
insmod /vendor/lib64/cellusermodeinterconnect.sys
insmod /vendor/lib64/cellnetmon.sys

# Initialize RIL
log -t simunlock "Initializing RIL"
qmicli -d /dev/qcril_radio --uim-get-card-status --timeout=30

# Wait for modem to be fully initialized
sleep 2

# Execute unlock sequence
log -t simunlock "Starting SIM unlock process"

# Get card status
CARD_STATUS=$(qmicli -d /dev/qcril_radio --uim-get-card-status)
if [ $? -ne 0 ]; then
    log -t simunlock "Failed to get card status"
    exit 1
fi

# Check if SIM is locked
if echo "$CARD_STATUS" | grep -q "PIN1 state: 'blocked'"; then
    log -t simunlock "SIM is PIN locked, attempting unlock"
    
    # Network unlock sequence
    qmicli -d /dev/qcril_radio --uim-depersonalization="network" --timeout=30
    if [ $? -ne 0 ]; then
        log -t simunlock "Network unlock failed"
        exit 1
    fi
    
    # Verify PIN1
    qmicli -d /dev/qcril_radio --uim-verify-pin=PIN1 --timeout=30
    if [ $? -ne 0 ]; then
        log -t simunlock "PIN verification failed"
        exit 1
    fi
fi

# Get card status again to verify
CARD_STATUS=$(qmicli -d /dev/qcril_radio --uim-get-card-status)
if echo "$CARD_STATUS" | grep -q "PIN1 state: 'enabled-verified'"; then
    log -t simunlock "SIM unlock successful"
else
    log -t simunlock "SIM unlock verification failed"
    exit 1
fi

# Set proper permissions
chmod 0644 /dev/qcril_radio
chown radio:radio /dev/qcril_radio

# Set proper permissions for RIL devices
chmod 0644 /dev/msril
chown radio:radio /dev/msril
chmod 0644 /dev/cellmux
chown radio:radio /dev/cellmux
chmod 0644 /dev/cellusermode
chown radio:radio /dev/cellusermode
chmod 0644 /dev/cellnetmon
chown radio:radio /dev/cellnetmon

log -t simunlock "SIM unlock process completed"