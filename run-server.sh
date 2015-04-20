#!/bin/sh

if [ $# -ne 3 ]
then
	echo "USAGE: $0 'PACKET_LOSS_RATE' 'PACKET_ERR_RATE' target_host"
	exit
fi
echo "export PACKET_LOSS_RATE=$1"
export PACKET_LOSS_RATE=$1
echo "export PACKET_ERR_RATE=$2"
export PACKET_ERR_RATE=$2
./tserver $3