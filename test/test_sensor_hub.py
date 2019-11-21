#!/usr/bin/env python

import sys
import pymodbus.client.sync
import json

if len(sys.argv) > 2:
    host = sys.argv[1]
    port = int(sys.argv[2])
elif len(sys.argv) > 1:
    host = 'localhost'
    port = int(sys.argv[1])
else:
    host = 'localhost'
    port = 16502

client = pymodbus.client.sync.ModbusTcpClient(host, port)

def print_device(device):
    print()
    print('Device: %d' % device)
    resp = client.read_input_registers(0, 40, unit=device)
    for reg in resp.registers:
        print(reg)
    resp = client.read_input_registers(20000, 40, unit=device)
    for reg in resp.registers:
        print(reg)

print_device(0)
print_device(1)
client.close()
