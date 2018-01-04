#!/system/bin/sh

if [[ ! -f /sns/cal/calb && -f /vendor/etc/sensors/sensor_def_variable.conf ]]; then
    rm /sns/cal/sns.reg
    touch /sns/cal/calb
fi

exit 0
