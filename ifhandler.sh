#!/bin/bash

function ip_down
{
    eval '/sbin/ifdown ${iface}'
}

function ip_up
{
    eval '/sbin/ifup ${iface}'
}

if [ $# -lt 1 ]; then
   echo "Interface required"
   exit 1
fi
iface=$1
uuid=$(xenstore-read $(/usr/bin/xenstore-read vm)/linkwatch)

# Capture stdout on fd 3
exec 3< <(/usr/bin/xenstore-watch /vm/${uuid}/link)
read <&3 path
state=$(/usr/bin/xenstore-read ${path})
echo "state is $state"
while read path; do
    new_state=$(/usr/bin/xenstore-read ${path})
    if ((($new_state ^ $state) == 1)); then
        state=$new_state
        if [[ $state -eq 1 ]]; then
            ip_up
        else
            ip_down
        fi
        eval '/usr/bin/ipsec-check.sh'
    fi

done <&3

# Close the descriptor
exec 3<&-

