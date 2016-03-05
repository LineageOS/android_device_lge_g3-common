#!/system/bin/sh

if [[ ! -f /sns/cal/updt && -f /system/etc/sensors/sensor_def_variable.conf ]]; then
    rm /sns/cal/sns.reg
    touch /sns/cal/updt
fi

exit 0