#!/usr/bin/env python

import sys

data = sys.argv[1].split(',')


nums = [int(i, 16) for i in data[2:]]

if nums[1] + 3 != len(nums):
    print("Lenght Failure!")
    sys.exit(1)

if (sum(nums) & 0xFF) != 1:
    print("Checksum Failure!")
    sys.exit(1)

print("OK!")
