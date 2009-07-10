#!/usr/bin/env python
#This script generated hundreds of random keypresses per second,
#  and sends them to the lyx window
#It requires xvkbd and wmctrl
#It generates a log of the KEYCODES it sends as development/keystest/out/KEYCODES

import random
import os 
import re
import sys

print "Beginning keytest.py"


class CommandSource:
	def __init__(self):
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

		self.keycode=keycode;
		self.count=0;
		self.count_max=1999;

	def getCommand(self):
		self.count=self.count+1;
		#if self.count > self.count_max:
		if self.count%200==0:
			#self.count=0
			return ("RaiseLyx")
		elif self.count > self.count_max:
			os._exit(0)
		else:
			keystr=""
			for k in range(1,2):
				keystr=keystr+self.keycode[random.randint(1,len(self.keycode))-1]
			return "KK: "+keystr

class CommandSourceFromFile(CommandSource):
	def __init__(self,filename,p):
		self.infile=open(filename,'r')
		self.lines=self.infile.readlines()
		self.p=p
		self.i=0
		self.count=0
		self.loops=0
		#Now we start randomly dropping lines, which we hope are redundant
		#p is the probability that any given line will be removed
		if (p>0):
			#The next couple of lines are to ensure that at least one line is dropped
			drop=random.randint(0,len(self.lines)-1)
			del self.lines[drop]
			p=p-(1/len(self.lines))
			j=0
			origlines=self.lines
			self.lines=[];
			for l in origlines:
				if random.uniform(0,1) < self.p:
					print "Randomly dropping line "+l+"\n"
				else:
					self.lines.append(l)
			print "LINES\n"
			print self.lines
			sys.stdout.flush()
			os.system("sleep 2")
		
	def getCommand(self):
		if self.i >= len(self.lines):
			if self.count >= 100 or self.loops>1:
				os.system("sleep 1")
				os._exit(0)
			else:
				self.loops=self.loops+1
				self.i=0
				return("Loop")
		line=self.lines[self.i]
		print "Line read: <<"+line+">>\n"
		self.count=self.count+1
		self.i=self.i+1
		return(line.rstrip())

def sendKeystring(keystr,LYX_PID):
	print "sending keystring "+keystr+"\n"
	if not re.match(".*\w.*", keystr):
 		print ("print ."+keystr+".\n")
		keystr="a"
	os.system("while ( test -e /proc/$LYX_PID/status && ! grep 'tate.*[(]sleeping[)]' /proc/$LYX_PID/status); do echo -n . ; sleep 0.02; done")
	cmd="xvkbd -xsendevent -text '"+keystr+"';sleep 0.03"
	sys.stdout.flush()
        os.system(cmd)
	sys.stdout.flush()

def RaiseWindow():
	os.system("echo x-session-manager PID: $X_PID.")
	os.system("echo x-session-manager open files: `lsof -p $X_PID | grep ICE-unix | wc -l`")
	os.system("wmctrl -l | ( grep '"+lyx_window_name+"' || ( killall lyx ; sleep 1 ; killall -9 lyx ))")
	os.system("wmctrl -R '"+lyx_window_name+"' ;sleep 0.1")
	
			
lyx_pid=os.environ.get("LYX_PID")
print("lyx_pid: "+lyx_pid+"\n");
infilename=os.environ.get("KEYTEST_INFILE")
outfilename=os.environ.get("KEYTEST_OUTFILE")
max_drop=os.environ.get("MAX_DROP")
lyx_window_name=os.environ.get("LYX_WINDOW_NAME");

file_new_command=os.environ.get("FILE_NEW_COMMAND");
if file_new_command is None:
	file_new_command="\Afn"

ResetCommand=os.environ.get("RESET_COMMAND");
if ResetCommand is None:
	ResetCommand="\[Escape]\[Escape]\[Escape]\[Escape]"+file_new_command
	#ResetCommand="\[Escape]\[Escape]\[Escape]\[Escape]\Cw\Cw\Cw\Cw\Cw\Afn"

if lyx_window_name is None:
	lyx_window_name="LyX";

print("outfilename: "+outfilename+"\n")
print("max_drop: "+max_drop+"\n")

if infilename is None:
	print("infilename is None\n")
	x=CommandSource()
	print ("Using x=CommandSource\n");
else:
	print("infilename: "+infilename+"\n")
	probability_we_drop_a_command=random.uniform(0,float(max_drop))
	print ("probability_we_drop_a_command: ")
	print '%s'%(probability_we_drop_a_command)
	print  "\n"
	x=CommandSourceFromFile(infilename,probability_we_drop_a_command)
	print ("Using x=CommandSourceFromFile\n");

outfile=open(outfilename,'w')

RaiseWindow()
sendKeystring("\Afn",lyx_pid)
write_commands=True;

while True:
	os.system("echo -n LOADAVG:; cat /proc/loadavg")
	c=x.getCommand()
	if (c=="Loop"):
		outfile.close()
		outfile=open(outfilename+'+','w')
		print ("Now Looping")
	outfile.writelines(c+'\n')
	outfile.flush()
	if c=="RaiseLyx":
		print ("Raising Lyx");
		RaiseWindow()
	elif c[0:4]=="KK: ":
  		if os.path.exists("/proc/"+lyx_pid+"/status"):
			sendKeystring(c[4:],lyx_pid)
		else:
   			os.system("killall lyx; sleep 2 ; killall -9 lyx")
   			print ("No path /proc/"+lyx_pid+"/status, exiting")
			os._exit(1)
	elif (c=="Loop"):
		RaiseWindow()
		sendKeystring(ResetCommand,lyx_pid)
	else:
		print ("Unrecognised Command '"+c+"'\n")
