# 6207  udevadm info -a -n /dev/ttyUSB0 | grep serial
# 6208  sudo nano /etc/udev/rules.d/99-myarduino.rules
### data:
# SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", SYMLINK+="stick-data"
# 6209  sudo udevadm control --reload-rules\nsudo udevadm trigger
# 6210  ls -l /dev/myarduino\n# /dev/myarduino -> ttyUSB0 (or whatever the current assignment is)
