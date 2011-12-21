#!/bin/python

import numpy
import sys

filename = sys.argv[1]
p = sys.argv[2]

varfile = "var.txt"
novarfile = "novar.txt"

data = []

for line in open(filename, 'r'):
   datum = int(line)
   data.append(datum)

if len(data) > 0:
   if all(x == data[0] for x in data):
      novarfd = open(novarfile, "a")
      novarfd.write("%s,%d\n" % (p,data[0]))
      novarfd.close()
   else:
      varfd = open(varfile, "a")
      avg = numpy.mean(data)
      sd = numpy.std(data)
      se = sd / numpy.sqrt(len(data))
      lower = avg - se * 1.96
      upper = avg + se * 1.96
      varfd.write("%s,%f,%f,%f\n" % (p, lower, avg, upper)) 
      varfd.close()   
