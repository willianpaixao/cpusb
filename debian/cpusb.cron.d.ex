#
# Regular cron jobs for the cpusb package
#
0 4	* * *	root	[ -x /usr/bin/cpusb_maintenance ] && /usr/bin/cpusb_maintenance
