#!/bin/bash

###
### Sample background worker script
### for linuxquestions.org written
### by Florian Harbich (user doc.nice)
###
### Free for use or modification, even if
### useless in this variant... 
###


#something is bad with the python, we get alot of zombies.  this is a kickstart script

LOOP=1
while [ $LOOP ]; do
killall -9 chromaStatus
sleep 300   
# 6 mins
done


