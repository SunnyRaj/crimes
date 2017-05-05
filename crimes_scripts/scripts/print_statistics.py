#!/usr/bin/python

#from __future__ import print_function
from collections import defaultdict
from numpy import mean
from decimal import Decimal
import re
import sys

def parse(filename):
    dirty_page_count = []
    timestamp = defaultdict(list)

    with open(filename, 'rb') as f:
        for line in f:
            # Skip data from live migration part
            if "starting checkpointing mechanism" in line:
                break
        for line in f:
            if "Dirty page count" in line:
                items = line.split(' ')
                dirty_page_count.append(int(items[-2]))
            if "Time at" in line:
                if "sr_suspend_start" in line:
                    s_time = re.search('[0-9]{12,16}', line)
                    suspend = s_time.group(0)
                else:
                    key = re.search('sr_(\w+)', line)
                    time = re.search('[0-9]{12,16}', line)
                    timestamp[key.group(0)].append(int(int(time.group(0)) - int(suspend)))

    print "Avg. statistics for this run:"
    print '%-5s %-20s %-4s' % ("#occ.", "Tag", "Timestamp")
    for k in sorted(timestamp, key=lambda k: float(mean(timestamp[k]))):
        ms = float(mean(timestamp[k])/1000000)
        l = len(timestamp[k])
        print '%-5s %-20s %-4s ms' % (l, k, round(ms,2))
    print '%-5s %-20s %-4s pages' % (len(dirty_page_count), "Dirty page count", mean(dirty_page_count))

def main():
    parse(sys.argv[1])

if __name__ == '__main__':
    main()
