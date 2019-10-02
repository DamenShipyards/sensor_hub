#!/usr/bin/env python

import os
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

if len(sys.argv) > 3:
    slave_id = int(sys.argv[3])
else:
    slave_id = 0

if len(sys.argv) > 4:
    sregs = sys.argv[4].split(',')
    for i in range(len(sregs) - 1, -1, -1):
        sreg = sregs[i]
        if '-' in sreg:
            lo, hi = sreg.split('-')
        sregs[i:i+1] = [str(j) for j in range(int(lo), int(hi) + 1)]
    regs = [int(i) for i in sregs]
else:
    regs = [0]

   
def clear():
    _ = os.system('clear' if os.name =='posix' else 'cls')

client = ModbusClient(ip, port)

min_regs = min(regs)
len_regs = max(regs) - min_regs + 1

while True:
    request = client.read_input_registers(min_regs, len_regs, unit=slave_id)
    if isinstance(request, Exception):
        print(request)
    else:
        for reg in regs:
            print('%.5d: %.5d' % (reg, request.registers[reg - min_regs]))
    time.sleep(1)
    clear()
