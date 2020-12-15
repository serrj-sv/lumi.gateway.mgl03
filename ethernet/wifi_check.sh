#!/bin/sh

loop=true
while $loop; do
	if [ "$(cat /sys/class/net/wlan0/operstate)" == "up" ]; then
		echo -e "wlan0 is up!"
		loop=false
	else
		echo -e "wlan0 is down!"
	fi

	sleep 5
done

/data/wifi_start.sh &

ifconfig wlan0 down
touch /tmp/dont_need_monitor_wpa_supplicant
killall wpa_supplicant
ifconfig eth0 up

while true; do
	# Keep script alive to prevent wifi respawn
	sleep 300
done
