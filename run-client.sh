#!/bin/sh

if [ $# -ne 4 ]
then
	echo "USAGE: $0 'PACKET_LOSS_RATE' 'PACKET_ERR_RATE' 'target_host' filename"
	exit
fi
echo "export PACKET_LOSS_RATE=$1"
export PACKET_LOSS_RATE=$1
echo "export PACKET_ERR_RATE=$2"
export PACKET_ERR_RATE=$2
./tclient $3 $4
