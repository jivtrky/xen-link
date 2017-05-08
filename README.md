# xen-link
Monitor Xen VIF link to take action on corresponding Eth0

File Description
config-xenstore.sh:

Each VM has 2 new properties "linkwatch" which is a pointer to the VM which 
it will monitor for link status changes and "link" which is the status of
it's own IP link state. This script adds and sets these properties in the 
xenstore specific to our configuration.

This script needs to be run on Dom0 before anything else. 
This may need to be looked at a little more.


ifnetlink daemon:
How to Build
gcc [-DDEBUG] -static ifnetlink.c match.c state.c -o ifnetlink
   OR
gcc [-DDEBUG] -m32 -static ifnetlink.c match.c state.c -o ifnetlink

The daemon takes one required argument, a regular expression of the interface(s)
to be monitored. 
i.e. - all vif's (vif), a specific one (vif6.0), every form of bridge (br)
ex: ifnetlink -i vif


ifnetlinkd.sh:
init.d script for the ifnetlink daemon


linknotify:
Put it in the /etc/monit.d/ directory of each node on which the ifnetlink daemon
is running.


ifhandler.sh:
script to be kicked off which will watch for changes in the
Xen store which are written by written by 'monit' triggered by changes in the vif or bridge 
monitored by the ifnetlink daemon. This script performs the appropriate action on the
corresponding eth0.  Right now it's just ifup/ifdown.

To start:
./ifhandler.sh eth0

