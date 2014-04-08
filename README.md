SimpleDriver
============

simple linux driver, that print "hello" in logs every t seconds.

Using
------------
dfd
____________

./reup.sh - rebuild driver and load it into kernel.
sudo ./setTimer t - set tick time (second). t=0 => timer stop.
dmesg | tail - show reult in logs.
