#!/usr/bin/env python3

import os
from glob import glob
from configparser import ConfigParser
from collections import OrderedDict

euid = os.geteuid()

if euid != 0:
    print('****')
    print('NOT running as root: will not be able to save')
    print('****')
    print()

def print_dict(d):
    for k, v in d.items():
        print(str(k) + ': ' + str(v))

new_config = OrderedDict()
config = ConfigParser()
config.read('/etc/sensor_hub/sensor_hub.conf')

def print_config():
    device_count = config.getint('devices', 'count', fallback=0)
    for i in range(device_count):
        device = 'device%d' % i
        kind = config.get(device, 'type')
        if kind == 'runwell_driver_serial':
            name = config.get(device, 'name') 
            conn = config.get(device, 'connection_string')
            new_config[device] = (name, conn)
            print(name + ": " + conn)

print('Current config:')
print_config()
print()
print('Available drivers:')
drivers = glob('/dev/sensor_hub/runwell_*')
numbered_drivers = OrderedDict()
for i, d in enumerate(drivers):
    numbered_drivers[str(i + 1)] = d

print_dict(numbered_drivers)

print()
for k, v in new_config.items():
    selectable = '[' + '|'.join(numbered_drivers.keys()) + ']'
    i = 'x'
    while i not in numbered_drivers:
        print('Select new value for: %s, %s' % (v[0], selectable))
        i = input()
    conn = numbered_drivers[i] + ':115200'
    config.set(k, 'connection_string', conn)
    del(numbered_drivers[i])

print()
print('New config:')
print_config()
print()
answer = 'x'
while answer not in ['y', 'n']:
    print('Do you want to save this config? [y|n]')
    answer = input()

if answer == 'y':
    with open('/etc/sensor_hub/sensor_hub.conf', 'w') as f:
        config.write(f)
    print('Restarting Sensor Hub')
    os.system('systemctl restart sensor_hub')
else:
    print('Configuration not saved')
