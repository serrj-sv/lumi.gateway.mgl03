# Updating mgl03 gateway firmware from telnet
Telnet must be opened on the gateway (via custom component from [@AlexxIT](https://github.com/AlexxIT/XiaomiGateway3/) or php-miio/python-miio).
You need telnet client like putty or other.
You can find out IP of the gateway in MiHome or on your router.
Login - "admin", no password.

<img src="https://gist.githubusercontent.com/zvldz/b40b4873e3c4c1a64ac536e8ce5dbdad/raw/screenshot_telnet1.png" width="400">

<img src="https://gist.githubusercontent.com/zvldz/b40b4873e3c4c1a64ac536e8ce5dbdad/raw/screenshot_telnet2.png" width="400">

# The easy way
Go to telnet session on gateway and run commands:
```sh
curl -s -k -L -o /tmp/update.sh https://gist.github.com/zvldz/b40b4873e3c4c1a64ac536e8ce5dbdad/raw/mgl03_update.sh && sh /tmp/update.sh
```
You will need to select firmware version.

If you are using the [XiaomiGateway3](https://github.com/AlexxIT/XiaomiGateway3) component.
For recommended firmware, see [https://github.com/AlexxIT/XiaomiGateway3/wiki](https://github.com/AlexxIT/XiaomiGateway3/wiki).

<img src="https://gist.githubusercontent.com/zvldz/b40b4873e3c4c1a64ac536e8ce5dbdad/raw/screenshot_telnet_script.png" width="768">

If you see something like in screenshot, all is ok - you have updated gateway.
If you used putty, window will close after rebooting gateway. Make sure there are no errors.

You don't need to read any more.

# The hard way (way of the warrior)

## Turning on ftp
### Via custom_component from [@AlexxIT](https://github.com/AlexxIT/XiaomiGateway3/)
Go to "Developer Tools/SERVICES" in Home Assistant.

And run service:
```
Service: remote.send_command
Entity: remote.0x680ae2fffe266ed5_pair (for example)

Service Data (YAML, optional):
entity_id: remote.0x680ae2fffe266ed5_pair
command: ftp
```
<img src="https://gist.githubusercontent.com/zvldz/b40b4873e3c4c1a64ac536e8ce5dbdad/raw/screenshot_ha.png" width="400">

### Manual mode
To start the ftp-server you need to log into gateway via telnet and execute the commands:
```sh
curl -k -o /data/busybox https://busybox.net/downloads/binaries/1.21.1/busybox-mipsel && chmod +x /data/busybox                                                            
/data/busybox tcpsvd -vE 0.0.0.0 21 /data/busybox ftpd -w &
```

<img src="https://gist.githubusercontent.com/zvldz/b40b4873e3c4c1a64ac536e8ce5dbdad/raw/screenshot_telnet3.png" width="758">

## Copying files via ftp to gateway
Download modified firmware from [firmware folder](https://github.com/serrj-sv/lumi.gateway.mgl03/tree/main/firmware/custom).

If you are using the [XiaomiGateway3](https://github.com/AlexxIT/XiaomiGateway3) component.
For recommended firmware, see [https://github.com/AlexxIT/XiaomiGateway3](https://github.com/AlexxIT/XiaomiGateway3).

For example [mgl03_1.4.7_0065_mod20201211.zip](https://github.com/serrj-sv/lumi.gateway.mgl03/blob/main/firmware/custom/mgl03_1.4.7_0065_mod20201211/mgl03_1.4.7_0065_mod20201211.zip).

Unzip archive mgl03_1.4.7_0065_mod20201211.zip.

You need ftp client like "FileZilla/WinSCP/Total Commander" etc.

Copy files linux_1.4.7_0065.bin and root_1.4.7_0065_mod20201211.bin to gateway folder /tmp.

Copy file full_ble_1.4.7_0065.gbl with filename full.gbl to /data/firmware (directory /data/firmware needs to be created).

## Starting update
Go back to telnet session on gateway and run commands:
```sh
fw_update /tmp/linux_1.4.7_0065.bin
fw_update /tmp/root_1.4.7_0065_mod20201211.bin
reboot
```

<img src="https://gist.githubusercontent.com/zvldz/b40b4873e3c4c1a64ac536e8ce5dbdad/raw/screenshot_telnet4.png" width="677">

After first reboot, gateway will reboot again to update ble firmware.
If you want to make sure if ble firmware has been updated, check for /data/firmware/full.gbl file, it should not exist.
If not, you can update the BLE firmware manually.
```sh 
run_ble_dfu.sh /dev/ttyS1 /data/firmware/full.gbl 123 1
``` 

All copied files will be deleted automatically.

In case of major changes between versions of updated firmware, you will most likely need to reset gateway.
