#!/bin/bash

sudo sysctl net.ipv4.ip_forward=1
sudo iptables --append FORWARD --in-interface wlp4s0 --out-interface enp0s20f0u2 -m state --state RELATED,ESTABLISHED -j ACCEPT
sudo iptables --append FORWARD --in-interface enp0s20f0u2 --out-interface wlp4s0 -j ACCEPT
sudo iptables --table nat --append POSTROUTING --out-interface wlp4s0 -j MASQUERADE
