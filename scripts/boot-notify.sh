#!/bin/sh
#
# $Id: boot-notify.sh,v 1.1 2002/07/04 21:42:50 root Exp $
# $Template: 1.0 $
#

echo_name () {
    echo -n " `basename $0 | awk '{print a[split($1, a, ".")-1]}'`"
}

notify () {
    echo "$1" | mail -s "$1" root && echo_name
}

case "$1" in
start|"")
	notify "`hostname` (`uname -sr`) is booted up"
	;;
stop)
	notify "`hostname` is shutting down"
	# we need the sleep here to allow sendmail to send letter,
	# if it is possible, otherwise it will be killed by shutdown scripts.
	sleep 10
	;;
*)
	echo "Usage: `basename $0` {start|stop}" >&2
	;;
esac

exit 0
