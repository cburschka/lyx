#! /usr/bin/python3
# -*- coding: utf-8 -*-

# file lyxpaperview.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Jürgen Spitzmüller
# This draws on a bash script and conceptual idea by Pavel Sanda
# Full author contact details are available in file CREDITS

# This script searches the home directory for a PDF or PS
# file with a name containing year and author. If found,
# it opens the file in a viewer. 

import getopt, os, sys, subprocess

pdf_viewers = ('pdfview', 'kpdf', 'okular', 'qpdfview --unique',
               'evince', 'xreader', 'kghostview', 'xpdf', 'SumatraPDF',
               'acrobat', 'acroread', 'mupdf',
               'gv', 'ghostview', 'AcroRd32', 'gsview64', 'gsview32')

ps_viewers = ("kghostview", "okular", "qpdfview --unique",
              "evince", "xreader", "gv", "ghostview -swap",
              "gsview64", "gsview32")

def message(message):
    sys.stderr.write("lyxpaperview: %s\n" % message)

def error(message):
    sys.stderr.write("lyxpaperview error: %s\n" % message)
    exit(1)

def usage(prog_name):
    msg = "Usage: %s [-v pdfviewer] [-w psviewer] titletoken-1 [titletoken-2] ... [titletoken-n]\n" \
          "    Each title token must occur in the filename (at an arbitrary position).\n" \
          "    You might use quotes to enter multi-word tokens"
    return  msg % prog_name

# Copied from lyxpreview_tools.py
# PATH and PATHEXT environment variables
path = os.environ["PATH"].split(os.pathsep)
extlist = ['']
if "PATHEXT" in os.environ:
    extlist += os.environ["PATHEXT"].split(os.pathsep)
extlist.append('.py')

def find_exe(candidates):
    global extlist, path

    for command in candidates:
        prog = command.split()[0]
        for directory in path:
            for ext in extlist:
                full_path = os.path.join(directory, prog + ext)
                if os.access(full_path, os.X_OK):
                    # The thing is in the PATH already (or we wouldn't
                    # have found it). Return just the basename to avoid
                    # problems when the path to the executable contains
                    # spaces.
                    if full_path.lower().endswith('.py'):
                        return command.replace(prog, '"%s" "%s"'
                            % (sys.executable, full_path))
                    return command

    return None


def find_exe_or_terminate(candidates):
    exe = find_exe(candidates)
    if exe == None:
        error("Unable to find executable from '%s'" % " ".join(candidates))

    return exe

def find(args, path):
    if os.name != 'nt':
        # use locate if possible (faster)
        if find_exe(['locate']):
            p1 = subprocess.Popen(['locate', '-i', args[0].lower()], stdout=subprocess.PIPE)
            px = subprocess.Popen(['grep', '-Ei', '\.pdf$|\.ps$'], stdin=p1.stdout, stdout=subprocess.PIPE)
            for arg in args:
               if arg == args[0]:
                   # have this already
                   continue
               px = subprocess.Popen(['grep', '-i', arg], stdin=px.stdout, stdout=subprocess.PIPE)
            p4 = subprocess.Popen(['head', '-n 2'], stdin=px.stdout, stdout=subprocess.PIPE)
            p1.stdout.close()
            output = p4.communicate()
            return output[0].decode("utf8")[:-1]# strip trailing '\n'
     # FIXME add something for windows as well?
     # Maybe dir /s /b %WINDIR%\*author* | findstr .*year.*\."ps pdf"

    for root, dirs, files in os.walk(path):
        for fname in files:
            lfname = fname.lower()
            if lfname.endswith(('.pdf', '.ps')):
                caught = True
                for arg in args:
                    if lfname.find(arg.lower()) == -1:
                        caught = False
                        break
                if caught:
                    return os.path.join(root, fname)
    return ""

def main(argv):
    progname = argv[0]
    
    opts, args = getopt.getopt(sys.argv[1:], "v:w:")
    pdfviewer = ""
    psviewer = ""
    for o, v in opts:
      if o == "-v":
        pdfviewer = v
      if o == "-w":
        psviewer = v
    
    if len(args) < 1:
      error(usage(progname))

    result = find(args, path = os.environ["HOME"])
    if result == "":
        message("no document found!")
        exit(2)
    else:
        message("found document %s" % result)

    viewer = ""
    if result.lower().endswith('.ps'):
        if psviewer == "":
            viewer = find_exe_or_terminate(ps_viewers)
        else:
            viewer = psviewer
    else:
        if pdfviewer == "":
           viewer = find_exe_or_terminate(pdf_viewers)
        else:
            viewer = pdfviewer
    
    subprocess.call([viewer, result])
    
    exit(0)

if __name__ == "__main__":
    main(sys.argv)
