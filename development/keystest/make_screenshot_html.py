#!/usr/bin/python
# -*- coding: utf-8 -*-
import re
import os
import sys

# sec="1250005988"
# dir='.'

# if len(sys.argv) > 1:

wdir = sys.argv[1]
sec = sys.argv[2]
base_filename = sec + '.s'
reGetNumber = re.compile(base_filename + '(\d*).png')

largest_screenshot_number = -1

for f in os.listdir(wdir):

    # print f

    m = reGetNumber.match(f)
    if m:

        # print f
        # print m.groups(1)[0]

        n = int(m.groups(1)[0])
        largest_screenshot_number = max(largest_screenshot_number, n)

# print largest_screenshot_number

keycodes = ['', 'KK: \Afn']
keycodes.extend(open(wdir + '/' + sec + '.KEYCODEpure'))
keycodes.extend(open(wdir + '/' + sec + '.KEYCODEpure+'))


def Highlight_Keycode_i(keycodes, i):
    s = ''
    for (j, k) in enumerate(keycodes):

        # print k

        if k[0:4] == 'KK: ':
            k = k[4:]
        if j == i:
            s = s + '<b>' + k + '</b>'
        else:
            s = s + k
    return s


print '<html>'
print '<title>Keytest Screenshots</title>'
for i in range(1, largest_screenshot_number + 1):
#for (i,k) in enumerate(keycodes)[1:]:
    k = keycodes[i]
    print '<a name='+str(i)+'></a>',
    print '<a href="#0">|&lt;</a> <a href="#'+str(max(i-1, 0))+'">&lt;&lt;</a>',
    print '<a href="#'+str(i+1)+'">&gt;&gt;</a>',
    print '<a href="#'+str(largest_screenshot_number)+'">&gt;|</a>',
    #print k.strip(),
    print Highlight_Keycode_i(keycodes, i),
    print '<br><img src="'+sec+'.s'+str(i)+'.png"><br>'
for k in keycodes[largest_screenshot_number+1:]:
    print k+'<br>'
print '<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>'
#print '</html>'