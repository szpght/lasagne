#!/usr/bin/env python3
"""This tool calculates indices in page tables for given addresss"""

from sys import argv
import sys

def convert(addr):
  try:
    addr = int(addr, base=0)
  except ValueError:
    print("Incorrect value\n")
    return

  if addr > 0x7fffffffffff and addr < 0xffff800000000000 or addr >= 2**64:
    print("Non-canonical address\n")
    return
  
  org = addr
  if addr >= 0xffff800000000000:
    addr -= 0xffff800000000000 - (0x7fffffffffff+1)

  l4 = addr // 2**39
  remainder = addr % 2**39

  l3 = remainder // 2**30
  remainder = remainder % 2**30

  l2 = remainder // 2**21
  remainder %= 2**21

  l1 = remainder // 2**12
  addr4 = org - org % 2**39
  addr3 = org - org % 2**30
  addr2 = org - org % 2**21
  addr1 = org - org % 2**12

  print("TABLE\t\tINDEX\tPAGE START ADDRESS\tPAGE START ADDRESS (COPYABLE)")
  print("Level 4\t\t%d\t%08X %08X\t0x%016X" % (l4, addr4 >> 32, addr4 % 2**32, addr4))
  print("Level 3\t\t%d\t%08X %08X\t0x%016X" % (l3, addr3 >> 32, addr3 % 2**32, addr3))
  print("Level 2\t\t%d\t%08X %08X\t0x%016X" % (l2, addr2 >> 32, addr2 % 2**32, addr2))
  print("Level 1\t\t%d\t%08X %08X\t0x%016X" % (l1, addr1 >> 32, addr1 % 2**32, addr1))
  print()
 
if len(argv) > 1:
  convert(argv[1])
else:
  while (True):
    try:
      addr = input()
      convert(addr)
    except KeyboardInterrupt:
      sys.exit(0)
    except ValueError:
      pass

