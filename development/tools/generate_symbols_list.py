#!/usr/bin/python
import sys,string,re,os

def is_prefix(a, b):
    return a[:len(b)] == b

def get_code(code, font):
    if code < 10:
	return code+161
    elif code < 32:
	return code+163
    else:
	return code

font_names = {}
symbols = {}
xsymbols = {}

ignore_list = ["not", "braceld", "bracerd", "bracelu", "braceru",
	       "lmoustache", "rmoustache", "lgroup", "rgroup", "bracevert"]

def process(file):
    fh = open(file)
    lines = fh.readlines()
    fh.close()

    n = len(lines)
    for i in xrange(n):
	line = lines[i]
	next_line = ""
	if i+1 < n:
	    next_line = lines[i+1]

	# some entries are spread over two lines so we join the next line 
	# to the current one, (if current line contains a comment, we remove it)
	line = string.split(line,'%')[0]+next_line

	mo =  re.match(r'.*\\DeclareSymbolFont\s*\{(.*?)\}\s*\{(.*?)\}\s*\{(.*?)\}.*', line)
	if mo != None:
	    font_names[mo.group(1)] = mo.group(3)

	mo =  re.match(r'.*\\DeclareMath(Symbol|Delimiter)\s*\{?\\(\w*?)\}?\s*\{?\\(.*?)\}?\s*\{(.*?)\}\s*\{"(.*?)\}.*', line)
	if mo != None:
	    symbol = mo.group(2)
	    type = mo.group(3)
	    font = mo.group(4)
	    code = mo.group(5)
	else:
	    mo = re.match(r'.*\\edef\\(\w*?)\{.*?\{\\hexnumber@\\sym(.*?)\}(.*?)\}', line)
	    if mo != None:
		symbol = mo.group(1)
		type = "mathord"
		font = mo.group(2)
		code = mo.group(3)
	    
	if mo != None and symbol not in ignore_list:
	    mo2 = re.match(r'\s*\\def\\(.*?)\{', next_line)
	    if mo2 != None and is_prefix(symbol,mo2.group(1)):
		sys.stderr.write("%s -> %s\n" % (symbol, mo2.group(1)))
		symbol = mo2.group(1)

	    if font_names.has_key(font):
		font = font_names[font]

	    code = get_code(string.atoi(code, 16), font)
	    if code == 0:
		continue

	    xcode = 0
	    if xsymbols.has_key(symbol):
		xcode = xsymbols[symbol]
		del xsymbols[symbol]

	    if symbols.has_key(symbol):
		sys.stderr.write(symbol+ " exists\n")
		if code != symbols[symbol]:
		    sys.stderr.write("code is not equal!!!\n")
	    else:
		symbols[symbol] = code
		print "%-18s %-4s %3d %3d %-6s" % (symbol,font,code,xcode,type)


path = os.path.split(sys.argv[0])[0]
fh = open(os.path.join(path, "x-font"))
lines = fh.readlines()
fh.close()
for line in lines:
    x = string.split(line)
    symbol = x[0]
    code = string.atoi(x[1],16)
    xsymbols[symbol] = code

for file in sys.argv[1:]:
    print "# Generated from " + os.path.basename(file) + "\n"
    process(file)
    print

exceptions = [
    ("neq", "none", 0, 185, "mathrel"),
    ("textdegree", "none", 0, 176, "mathord"),
    ("cong", "none", 0, 64, "mathrel"),
    ("surd", "note", 0, 214, "mathord")
]

if xsymbols.has_key("leq"):
    sys.exit(0)

for x in exceptions:
    print "%-18s %-4s %3d %3d %-6s" % x
    if xsymbols.has_key(x[0]):
	del xsymbols[x[0]]

for symbol in xsymbols.keys():
    sys.stderr.write(symbol+"\n")
