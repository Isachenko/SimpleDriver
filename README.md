SimpleDriver
============

simple linux driver, that print "hello" in logs every t seconds.

Using
------------

- [x] ./reup.sh - rebuild driver and load it into kernel.
- [x] sudo ./setTimer t - set tick time (second). t=0 => timer stop.
- [x] dmesg | tail - show reult in logs.
