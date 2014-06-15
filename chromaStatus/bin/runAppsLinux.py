import threading
import subprocess
import os, signal
from time import sleep

class MyClass(threading.Thread):

    def __init__(self):
        self.stdout = None
        self.stderr = None
        threading.Thread.__init__(self)

    def run(self):

        while (1):
            print "starting a new process"
            p = subprocess.Popen('./chromaStatus'.split(),
                                 shell=False,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
            p.wait()
	    if p.poll() is None:
	    	print "killing process: %d" % p.pid
	    	os.kill(p.pid, signal.SIGTERM)

        

print "starting"

for x in range(0, 50):
    myclass = MyClass()
    myclass.start()


print "kicked it off now what?"
