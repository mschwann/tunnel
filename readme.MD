#Tunnel
I have behind-a-nat cluster, that I really wanna access - therefore, the project.

#Always blame others - skipping viface.

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



For removing links:
 sudo ip link delete tun0