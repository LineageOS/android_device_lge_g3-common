# Audio
PRODUCT_PROPERTY_OVERRIDES += \
    media.aac_51_output_enabled=true \
    av.offload.enable=false

PRODUCT_PROPERTY_OVERRIDES += \
    mm.enable.smoothstreaming=true

PRODUCT_PROPERTY_OVERRIDES += \
    persist.audio.fluence.voicecall=true \
    persist.audio.fluence.voicerec=false \
    persist.audio.fluence.speaker=true

PRODUCT_PROPERTY_OVERRIDES += \
    ro.qc.sdk.audio.fluencetype=fluence

PRODUCT_PROPERTY_OVERRIDES += \
    use.voice.path.for.pcm.voip=true

# Camera
PRODUCT_PROPERTY_OVERRIDES += \
    camera2.portability.force_api=1

# Display
PRODUCT_PROPERTY_OVERRIDES += \
    persist.hwc.mdpcomp.enable=true \
    ro.opengles.version=196608 \
    ro.sf.lcd_density=560

# GPS
PRODUCT_PROPERTY_OVERRIDES += \
    persist.gps.qc_nlp_in_use=1 \
    persist.loc.nlp_name=com.qualcomm.services.location \
    ro.gps.agps_provider=1 \
    ro.qc.sdk.izat.premium_enabled=0 \
    ro.qc.sdk.izat.service_mask=0x0

# NITZ
PRODUCT_PROPERTY_OVERRIDES += \
    persist.rild.nitz_plmn="" \
    persist.rild.nitz_long_ons_0="" \
    persist.rild.nitz_long_ons_1="" \
    persist.rild.nitz_long_ons_2="" \
    persist.rild.nitz_long_ons_3="" \
    persist.rild.nitz_short_ons_0="" \
    persist.rild.nitz_short_ons_1="" \
    persist.rild.nitz_short_ons_2="" \
    persist.rild.nitz_short_ons_3=""

# Qualcomm
PRODUCT_PROPERTY_OVERRIDES += \
    persist.timed.enable=true \
    ro.qualcomm.cabl=0 \
    ro.vendor.extension_library=/vendor/lib/libqti-perfd-client.so

# Radio
PRODUCT_PROPERTY_OVERRIDES += \
    persist.radio.add_power_save=1 \
    persist.radio.apm_sim_not_pwdn=1 \
    ro.ril.telephony.mqanelements=5

PRODUCT_PROPERTY_OVERRIDES += \
    persist.data.netmgrd.qos.enable=false \
    persist.data.qmi.adb_logmask=0 \
    ro.use_data_netmgrd=true

# Recovery
PRODUCT_PROPERTY_OVERRIDES += \
    ro.cwm.forbid_format=/boot,/firmware,/mpt,/persist,/persist-lg,/sns

# Sensors
PRODUCT_PROPERTY_OVERRIDES += \
    debug.qualcomm.sns.daemon=e \
    debug.qualcomm.sns.hal=e \
    debug.qualcomm.sns.libsensor1=e
