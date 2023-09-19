#Tunnel
I have behind-a-nat cluster, that I really wanna access - therefore, the project.

#Option one - get simpletun repo to work.

openvpn --mktun --dev tun0 --user zero
ip link set tun0 up
ip addr add 172.16.0.1/24 dev tun0
./simpletun -i tun0 -s


openvpn --mktun --dev tun0 --user zero
ip link set tun0 up
ip addr add 172.16.0.2/24 dev tun0
./simpletun -i tun0 -c 192.168.0.111


sudo sysctl -w net.ipv4.ip_forward=1
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE



#For removing links:
 sudo ip link delete tun0

# I used nebula too - looks kinda cool.
Current settings:
```
#Nebula nodes
nebula-cert sign -name "lighthouse.neb.zerus.org" -ip "172.16.0.1/16"
#Infrastructure hypervisors
nebula-cert sign -name "pve0.neb.zerus.org" -ip "172.16.1.1/16"
nebula-cert sign -name "pve1.neb.zerus.org" -ip "172.16.1.2/16"
nebula-cert sign -name "pve2.neb.zerus.org" -ip "172.16.1.3/16"
nebula-cert sign -name "pve3.neb.zerus.org" -ip "172.16.1.4/16"
#User-points
nebula-cert sign -name "pc.neb.zerus.org" -ip "172.16.2.1/16"
nebula-cert sign -name "lap.neb.zerus.org" -ip "172.16.2.2/16"
nebula-cert sign -name "work.neb.zerus.org" -ip "172.16.2.3/16"
#Endpoints
nebula-cert sign -name "ftp.neb.zerus.org" -ip "172.16.100.1/16"
```
