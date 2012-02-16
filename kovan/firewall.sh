#!/bin/bash

IPTABLES="/sbin/iptables"
IP6TABLES="/sbin/ip6tables"

#### Bridge Interface 
BRIDGE=br0
###  KVM Linux Server Interfaces 
MNG_IP="10.20.30.40"
MNG_IP6="2001:a98:dede:efe:fafa:1"

### System administrators net
MNG_NET="192.168.3.0/24"
MNG_NET6="2001:a98:dede:baba::/64"
KOVAN_NET="192.168.0.0"
KOVAN_NET6="2001:a98::"
### VNC Port Range
VNC_RANGE="5900:5920"
QUAGGA_RANGE="2601:2603"

#
# Flush (-F) all specific rules
#
$IPTABLES -F INPUT 
$IPTABLES -F FORWARD 
$IPTABLES -F OUTPUT 
$IPTABLES -F -t nat

#### Flush Rules For IPv6
$IP6TABLES -F INPUT
$IP6TABLES -F FORWARD
$IP6TABLES -F OUTPUT

### Accept Everyting 
$IPTABLES -P INPUT ACCEPT
$IPTABLES -P FORWARD ACCEPT
$IPTABLES -P OUTPUT ACCEPT

### Accept Everyting For IPv6 

$IP6TABLES -P INPUT ACCEPT
$IP6TABLES -P FORWARD ACCEPT
$IP6TABLES -P OUTPUT ACCEPT


### Allow management IP address SSH and VNC connection
$IPTABLES -A INPUT -p tcp -i $BRIDGE  -s $MNG_NET -d $MNG_IP -m multiport  --dports 22,$VNC_RANGE -m state --state NEW -j ACCEPT
$IP6TABLES -A INPUT -p tcp -i $BRIDGE  -s $MNG_NET6 -d $MNG_IP6  -m multiport --dports 22,$VNC_RANGE  -m state --state NEW -j ACCEPT
$IPTABLES -A INPUT -p udp -i $BRIDGE  -s $MNG_NET -d $MNG_IP -m multiport  --dports 22,$VNC_RANGE -m state --state NEW -j ACCEPT
$IP6TABLES -A INPUT -p udp -i $BRIDGE  -s $MNG_NET6 -d $MNG_IP6  -m multiport --dports 22,$VNC_RANGE  -m state --state NEW -j ACCEPT

### Allow management IP address QUAGGA connection
$IPTABLES -A INPUT -p tcp -i $BRIDGE  -s $MNG_NET -d $KOVAN_NET -m multiport  --dports $QUAGGA_RANGE -m state --state NEW -j ACCEPT
$IP6TABLES -A INPUT -p tcp -i $BRIDGE  -s $MNG_NET6 -d $KOVAN_NET6  -m multiport --dports $QUAGGA_RANGE  -m state --state NEW -j ACCEPT

### Deny all other connection  to SSH and VNC 
$IPTABLES -A INPUT -p tcp -i $BRIDGE -d $MNG_IP -m multiport  --dports 22,$VNC_RANGE  -m state --state NEW -j DROP
$IP6TABLES -A INPUT -p tcp -i $BRIDGE -d $MNG_IP6 -m multiport --dports 22,$VNC_RANGE  -m state --state NEW  -j DROP
$IPTABLES -A INPUT -p udp -i $BRIDGE -d $MNG_IP -m multiport  --dports 22,$VNC_RANGE  -m state --state NEW -j DROP
$IP6TABLES -A INPUT -p udp -i $BRIDGE -d $MNG_IP6 -m multiport --dports 22,$VNC_RANGE   -m state --state NEW -j DROP

### Deny connection to KOVAN quagga Ports
$IPTABLES -A INPUT -p tcp -i $BRIDGE -d $KOVAN_NET -m multiport  --dports $QUAGGA_RANGE  -m state --state NEW -j DROP
$IP6TABLES -A INPUT -p tcp -i $BRIDGE -d $KOVAN_NET6 -m multiport --dports 22,$QUAGGA_RANGE  -m state --state NEW  -j DROP
