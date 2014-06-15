import threading
import subprocess


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
        





print "starting"

for x in range(0, 60):
    myclass = MyClass()
    myclass.start()


print "kicked it off now what?"
