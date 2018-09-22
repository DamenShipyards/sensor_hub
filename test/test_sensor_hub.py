#!/usr/bin/env python

import sys
import pymodbus.client.sync
import json


client = pymodbus.client.sync.ModbusTcpClient('127.0.0.1', 10502)
resp = client.read_input_registers(0, 40, unit=0)
for reg in resp.registers:
    print(reg)
client.close()
