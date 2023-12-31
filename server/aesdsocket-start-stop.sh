#!/bin/sh

#--------------------------------------
# Author: Yusuf Abdulsttar
#--------------------------------------

case "$1" in
    start)
        echo "Starting aesdsocket"
        start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
        ;;
    stop)
        echo "Stopping aesdsocket"
        start-stop-daemon -K -n /usr/bin/aesdsocket
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
        ;;
esac
