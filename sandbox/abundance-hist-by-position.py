import sys

n = 0
countSum = [0]*255
countN = [0]*255

fd = open(sys.argv[1])
for n, line in enumerate(fd):
   if n % 100000 == 0:
      print >>sys.stderr, '...', n

   tok = line.split()

   for i in range(len(tok)):
      countSum[i] += int(tok[i])
      countN[i] += 1

y = [0.0]*len(countSum)

for i in range(len(countSum)):
   if countN[i]:
      y[i] = float(countSum[i]) / float(countN[i])

for n, i in enumerate(y):
   print n, i
