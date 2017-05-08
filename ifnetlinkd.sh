#! /bin/bash

PATH=/sbin:/bin:/usr/bin:/usr/sbin

DAEMON_PATH=/usr/bin
DAEMON=ifnetlink
DAEMON_OPTS=' -i vif'
PIDFILE=/var/run/ifnetlinkd.pid

test -x $DAEMON_PATH/$DAEMON || exit 0

# Source function library.
. /etc/init.d/functions

# Return exit status 1 if daemon is found
status(){

        if $(pidof $DAEMON > /dev/null); then
           (exit 1)
        else
           (exit 0)
        fi
}

startdaemon(){
        echo -n "Starting ifnetlink: "
        status
        if [ $? == 1 ]; then
            echo "Done"
        else
            $DAEMON_PATH/$DAEMON $DAEMON_OPTS
            status
            if [ $? == 1 ]; then
                echo "Done"
            else
                echo "Failed"
            fi
        fi
}
stopdaemon(){
        echo -n "Stopping ifnetlink: "
        status
        if [ $? == 0 ]; then
            echo "Done"
            exit 0
        else
            PID=$(pidof $DAEMON)
            kill -SIGKILL $PID
            status
            if [ $? == 0 ]; then
                echo "Done"
                exit 0
            else
                echo "Failed"
            fi
        fi
}

statusdaemon(){
        status
        if [ $? == 1 ]; then
            echo "Running"
            exit 0
        else
            echo "Not Running"
            exit 1
        fi
}

case "$1" in
  start)
        startdaemon
        ;;
  stop)
        stopdaemon
        ;;
  restart)
        stopdaemon
        sleep 1
        startdaemon
        ;;
  status)
        statusdaemon
        ;;
  *)
        echo "Usage: ntpd { start | stop | status | restart }" >&2
        exit 1
        ;;
esac

exit 0

