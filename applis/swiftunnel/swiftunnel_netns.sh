#!/bin/bash

#
# Author Emmanuel Lochin (ENAC)
#
# Script to test swiftunnel using network namespace.
#

NS="ns1"
VETH="veth1"
VPEER="vpeer1"
VETH_ADDR="10.1.1.1"
VPEER_ADDR="10.1.1.2"

TUN="tun0"
TUN_CLIENT="192.168.10.1"
TUN_SERVER="192.168.10.2"
SWIF_TUN="./swiftunnel"

# FEC parameters — must match the swiftunnel invocations below
K=7
W=50

# Netem impairment parameters
DELAY=20              # one-way delay in ms
LOSS=2               # loss percentage

cleanup() {
    kill $(jobs -p) 2>/dev/null || true
    wait 2>/dev/null || true
    sudo tc qdisc del dev $VETH root 2>/dev/null || true
    sudo ip -all netns delete  2>/dev/null || true
    sudo ip link del dev $VETH 2>/dev/null || true
    sudo ip link del dev $TUN  2>/dev/null || true
}
trap cleanup EXIT

sudo ip -all netns delete  2>/dev/null || true
sudo ip link del dev $VETH 2>/dev/null || true
sudo ip link del dev $TUN  2>/dev/null || true

sudo ip netns add $NS
sudo ip link add ${VETH} type veth peer name ${VPEER}
sudo ip link set ${VPEER} netns $NS
sudo ip addr add ${VETH_ADDR}/24 dev ${VETH}
sudo ip link set ${VETH} up
sudo ip netns exec $NS ip addr add ${VPEER_ADDR}/24 dev ${VPEER}
sudo ip netns exec $NS ip link set ${VPEER} up
sudo ip netns exec $NS ip link set lo up

sudo ip tuntap add ${TUN} mode tun
sudo ip link set ${TUN} up
sudo ip addr add ${TUN_CLIENT}/24 dev ${TUN}

sudo ip netns exec $NS ip tuntap add ${TUN} mode tun
sudo ip netns exec $NS ip link set ${TUN} up
sudo ip netns exec $NS ip addr add ${TUN_SERVER}/24 dev ${TUN}

echo "=== Tunnel FEC : k=${K}, window=${W} ==="
sudo ip netns exec $NS ${SWIF_TUN} -i ${TUN} -s -k ${K} -w ${W} &
sleep 1
${SWIF_TUN} -i ${TUN} -c ${VPEER_ADDR} -k ${K} -w ${W} &
sleep 1

sudo tc qdisc add dev $VETH root netem delay ${DELAY}ms loss ${LOSS}%
echo "=== Netem : delay=${DELAY}ms, loss=${LOSS}% ==="
