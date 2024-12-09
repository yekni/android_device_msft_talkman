# RIL
PRODUCT_PACKAGES += \
    libril \
    libreference-ril \
    librilutils \
    libqmi_cci \
    libqmi_common_so \
    libqmiservices \
    qmicli

# Radio Properties
PRODUCT_PROPERTY_OVERRIDES += \
    rild.libpath=/vendor/lib64/libril-qc-qmi-1.so \
    persist.radio.multisim.config=ssss \
    ro.telephony.default_network=9 \
    telephony.lteOnCdmaDevice=0 \
    persist.vendor.radio.apm_sim_not_pwdn=1 \
    persist.vendor.radio.custom_ecc=1 \
    persist.vendor.radio.rat_on=combine \
    persist.vendor.radio.sib16_support=1 