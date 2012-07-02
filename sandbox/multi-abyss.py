#! /usr/bin/env python
import sys, time
import subprocess

#COMMAND="ABYSS -k45 %(filename)s -o %(filename)s.k45"
COMMAND="abyss-pe k=45 n=5 in=\\'%(filename)s\\' name=%(filename)s.zzz"

N_PROCESSES = 4

filenames = sys.argv[1:]

running = []

while 1:
    while len(running) < N_PROCESSES:
        filename = filenames.pop(0)
        cmd = COMMAND % dict(filename=filename)
        p = subprocess.Popen(cmd, shell=True)
        print 'running:', cmd
        running.append(p)

    i = 0;
    while i < len(running):
        p = running[i]
        returncode = p.poll()
        if returncode is not None:
            print 'done! removing', p
            running.remove(p)
        else:
            i += 1

    time.sleep(1)

print '\n\n** done **\n'
