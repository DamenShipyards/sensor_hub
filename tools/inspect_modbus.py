#!/usr/bin/env python

import sys
import time
from pymodbus.client.sync import ModbusTcpClient as ModbusClient

if len(sys.argv) > 1:
    ip = sys.argv[1]
else:
    ip = '127.0.0.1'

if len(sys.argv) > 2:
    port = int(sys.argv[2])
else:
    port = 16502
   

client = ModbusClient(ip, port)

while True:
    request = client.read_input_registers(0x00, 100, unit=0)
    if isinstance(request, Exception):
        print(request)
    else:
        print(request.registers)
    time.sleep(4)
