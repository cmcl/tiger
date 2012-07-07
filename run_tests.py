
##import os
##for n in xrange(1000, 11000, 1000):
##    for i in range(5):
##        os.system("\"bin/parsertest\" tigertests/test{}.tig > output{}".format(n, i))
##        with open("output{}".format(n), "a+") as f:
##            with open("output{}".format(i), "r") as fr:
##                f.write(fr.readline())
##

from subprocess import Popen, PIPE


program = "bin/treetest"
name = program.split("/")[1]
results = set()
incorrect_programs = "9, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, \
23, 24, 25, 26, 28, 29, 31, 32, 33, 34, 35, 36, 40, 43, 45, 49".split(", ")
for n in xrange(1, 50):
    if str(n) in incorrect_programs:
        print "Skipping incorrect program", n
        continue
    else:
        print "Compiling program", n, "to IR..."
    p = Popen([program, "tigertests/test{0}.tig".format(n)],
               stdout=PIPE, stderr=PIPE)
    s = p.stderr.read()
    print s
    if n == 4:
        print p.stdout.read()
    if name in s:
        print "Test", n
    elif "Segmentation fault" in s:
        print "Test", n
    results.add(s)

for x in results:
    print x
