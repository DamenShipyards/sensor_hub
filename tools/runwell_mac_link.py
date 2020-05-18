#!/usr/bin/env python3

import os
import time
import sys
try:
    import serial
except ImportError:
    print('Failed: requires pyserial.')
    sys.exit(1)

if len(sys.argv) < 2:
    print('Usage: runwell_mac_link.py <path_to_serial_device>.')
    sys.exit(2)

dev = sys.argv[1]

s = serial.Serial(timeout=1.0)
s.baudrate = 115200
s.port = dev
try:
    s.open()
except Exception as e:
    print('Failed to connect to runwell on %s: %s' % (dev, e))
    sys.exit(3)
else:
    s.reset_input_buffer()
    s.write('a\n'.encode('utf-8'))
    s.flush()
    mac = s.read(size=17).decode('utf-8')
    if len(mac) != 17:
        print('Read operation timed out. Received only %d bytes' % len(mac))
        sys.exit(4)
    mac = mac.replace(':', '').replace(' ', '')
    mac_path = '/dev/sensor_hub/runwell_%s' % mac
    os.system('ln -sf "%s" "%s"' % (dev, mac_path))



