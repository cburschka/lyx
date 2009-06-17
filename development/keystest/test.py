#!/usr/bin/env python
#This script generated hundreds of random keypresses per second,
#  and sends them to the lyx window
#It requires xvkbd and wmctrl
#It generates a log of the KEYCODES it sends as development/keystest/out/KEYCODES

import random
import os 

keycode=["\[Left]",'\[Right]','\[Down]','\[Up]','\[BackSpace]','\[Delete]','\[Escape]']
keycode[:0]=keycode
keycode[:0]=keycode

keycode[:0]=['\\']

for k in range(97, 123):
  keycode[:0]=chr(k)

for k in range(97, 123):
  keycode[:0]=["\A"+chr(k)]

for k in range(97, 123):
  keycode[:0]=["\A"+chr(k)]

for k in range(97, 123):
  keycode[:0]=["\C"+chr(k)]


print (keycode[1])
print(keycode)
print (random.randint(1,len(keycode)))
for k in range(97, 123):
  print (keycode[random.randint(1,len(keycode))-1])

#Start a new file. We could also open a random Help file.
#os.system("wmctrl -R LyX && xvkbd -xsendevent -text '\Afn';sleep 1")
keystr="'\Afn'"
os.system("wmctrl -R LyX && xvkbd -xsendevent -text '"+keystr+"';sleep 1")
os.system("echo '"+keystr+"'")

while True:
  keystr=""
  for k in range(1,80):
	keystr=keystr+keycode[random.randint(1,len(keycode))-1]
  #output keystr before using, to make sure it is output before we are killed
  os.system("echo '"+keystr+"'")
  os.system("wmctrl -R LyX && xvkbd -xsendevent -text '"+keystr+"';sleep 1")
