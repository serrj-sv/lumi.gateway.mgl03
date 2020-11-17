* Based on official 1.4.7_0065, built by [@zvldz](https://github.com/zvldz)
* List of modifications:
  * enabled 'telnetd'
  * disabled gen password
  * removes password if exists
  * mosquitto port is open for access
  * run on startup '/data/run.sh'
  * added 'curl'
  * added 'dropbear' (to start add in '/data/run.sh' line '/bin/dropbear -R -B')
  * replaced 'silabs_ncp_bt' with modified version (works without internet)