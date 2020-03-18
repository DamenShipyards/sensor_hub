#!/usr/bin/env python3
from __future__ import print_function
import socket
import sys

port = int(sys.argv[1])

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
s.bind(('', port))
while True:
    data = s.recv(1024)
    print('Received: {0}'.format(data))
