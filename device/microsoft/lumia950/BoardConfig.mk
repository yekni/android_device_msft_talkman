# RIL
ENABLE_VENDOR_RIL_SERVICE := true
BOARD_PROVIDES_LIBRIL := true

# SELinux
BOARD_SEPOLICY_DIRS += \
    device/microsoft/lumia950/sepolicy/vendor

# Radio
TARGET_PROVIDES_QTI_TELEPHONY_JAR := true

# Device Path
TARGET_DEVICE := lumia950
PRODUCT_DEVICE := lumia950
PRODUCT_NAME := lumia950