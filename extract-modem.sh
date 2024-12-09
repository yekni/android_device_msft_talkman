#!/bin/bash

# Source and destination paths
MAINOS_PATH="./mainOS"  # Adjust this to your mainOS folder path
OUTPUT_BASE="./output"

# Create output directories
mkdir -p ${OUTPUT_BASE}/modem
mkdir -p ${OUTPUT_BASE}/qmi
mkdir -p ${OUTPUT_BASE}/radio
mkdir -p ${OUTPUT_BASE}/firmware

# Modem files
echo "Copying modem files..."
cp -v "${MAINOS_PATH}/Windows/System32/OEM/Modem/modem.mbn" "${OUTPUT_BASE}/modem/" 2>/dev/null || echo "modem.mbn not found"
cp -v "${MAINOS_PATH}/Windows/System32/OEM/Modem/modem_pr" "${OUTPUT_BASE}/modem/" 2>/dev/null || echo "modem_pr not found"
cp -v "${MAINOS_PATH}/Windows/System32/OEM/Modem/qcril.db" "${OUTPUT_BASE}/modem/" 2>/dev/null || echo "qcril.db not found"
cp -v "${MAINOS_PATH}/Windows/System32/OEM/Modem/mba.mbn" "${OUTPUT_BASE}/modem/" 2>/dev/null || echo "mba.mbn not found"
cp -v "${MAINOS_PATH}/Windows/System32/OEM/Modem/modem.b*" "${OUTPUT_BASE}/modem/" 2>/dev/null || echo "modem.b* not found"
cp -v "${MAINOS_PATH}/Windows/System32/OEM/Modem/modemr.jsn" "${OUTPUT_BASE}/modem/" 2>/dev/null || echo "modemr.jsn not found"

# QMI Configuration
echo "Copying QMI files..."
mkdir -p "${OUTPUT_BASE}/qmi"
cp -rv "${MAINOS_PATH}/Windows/System32/OEM/QMI/"* "${OUTPUT_BASE}/qmi/" 2>/dev/null || echo "QMI files not found"

# Radio files
echo "Copying radio files..."
mkdir -p "${OUTPUT_BASE}/radio"
cp -rv "${MAINOS_PATH}/Windows/System32/OEM/Radio/"* "${OUTPUT_BASE}/radio/" 2>/dev/null || echo "Radio files not found"

# Create device tree directories
DEVICE_FIRMWARE_PATH="device/microsoft/lumia950/firmware"
mkdir -p "${DEVICE_FIRMWARE_PATH}/modem"
mkdir -p "${DEVICE_FIRMWARE_PATH}/qmi"
mkdir -p "${DEVICE_FIRMWARE_PATH}/radio"

# Copy files to device tree
echo "Copying files to device tree..."
cp -rv "${OUTPUT_BASE}/modem/"* "${DEVICE_FIRMWARE_PATH}/modem/"
cp -rv "${OUTPUT_BASE}/qmi/"* "${DEVICE_FIRMWARE_PATH}/qmi/"
cp -rv "${OUTPUT_BASE}/radio/"* "${DEVICE_FIRMWARE_PATH}/radio/"

echo "Done! Check the output folders for extracted files."
echo "Files have been copied to device tree at: ${DEVICE_FIRMWARE_PATH}"

# List found files
echo -e "\nExtracted files:"
find "${OUTPUT_BASE}" -type f