##POWER2_MON
### A Simple Battery Warning System


###Summary
-------

**power2_mon** is an application that checks the battery status
	and if charged or low will pop up a small X window. The only dependency 
	is Xlib.


###Usage
-----

**power2_mon** will continually poll the battery status(every 5 min by default).
    It will check the status, if the battery is below MIN_PERCENT, above MAX_PERCENT
    or charged it pops up a small window (click anywhere in the window to close it).

**power2_mon** should be launched from ~/.xinitrc or the window
    managers' autostart facility.

**e.g.**

    * [ "`ps -C power_mon -o pid=`" != "" ] || \
    * ~/build/power2_mon/power2_mon &

###Installation
------------

Need Xlib, then:

    Edit the power2_mon.c file to point to the relevant file in
    /sys/class/power2_supply.
    Check the three searched terms and defines are relevant or edit to suit.
**e.g.**

      * #define SYS_FILE "/sys/class/power_supply/BAT0/uevent"
      * #define MIN_PERCENT 34
      * #define MAX_PERCENT 90
      * #define SEARCHTERM1 "POWER_SUPPLY_STATUS"
      * #define SEARCHTERM2 "POWER_SUPPLY_CHARGE_FULL="
      * #define SEARCHTERM3 "POWER_SUPPLY_CHARGE_NOW="
      * static unsigned int SLEEP_TIME = 300;

    $ make
    # make install
    $ make clean


###Bugs
----

[ * No bugs for the moment ;) (I mean, no importants bugs ;)]


###Todo
----

  * I'll think of something

