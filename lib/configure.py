#! /usr/bin/env python
#
# file configure.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# \author Bo Peng
# Full author contact details are available in file CREDITS.

# This is an experimental version of the configure script, written
# in Python. 

import sys, os, re, shutil, glob

# compatibility with python 2.2
if sys.version_info[:2] == (2, 2):
    __builtins__.True = (1 == 1)
    __builtins__.False = (1 == 0)
    def bool(value):
        """Demote a value to 0 or 1, depending on its truth value

        This is not to be confused with types.BooleanType, which is
        way too hard to duplicate in 2.1 to be worth the trouble.
        """
        return not not value
    __builtins__.bool = bool
    del bool
# end compatibility chunk


class Tee:
    ''' Writing to a Tee object will write to all file objects it keeps.
        That is to say, writing to Tee(sys.stdout, open(logfile, 'w')) will
        write to sys.stdout as well as a log file.
    '''
    def __init__(self, *args):
        self.files = args

    def write(self, data):
        for f in self.files:
            result = f.write(data)
        return result

    def writelines(self, seq):
        for i in seq:
            self.write(i)


def writeToFile(filename, lines, append = False):
    " utility function: write or append lines to filename "
    if append:
        file = open(filename, 'a')
    else:
        file = open(filename, 'w')
    file.write(lines)
    file.close()


def addToRC(lines):
    ''' utility function: shortcut for appending lines to outfile
        add newline at the end of lines.
    '''
    if lines.strip() != '':
        writeToFile(outfile, lines + '\n', append = True)


def removeFiles(filenames):
    '''utility function: 'rm -f'
        ignore errors when file does not exist, or is a directory.
    '''
    for file in filenames:
        try:
            os.remove(file)
        except:
            pass


def cmdOutput(cmd):
    '''utility function: run a command and get its output as a string
        cmd: command to run
    '''
    fout = os.popen(cmd)
    output = fout.read()
    fout.close()
    return output.strip()


def setEnviron():
    ''' I do not really know why this is useful, but we might as well keep it.
        NLS nuisances.
        Only set these to C if already set.  These must not be set unconditionally
        because not all systems understand e.g. LANG=C (notably SCO).
        Fixing LC_MESSAGES prevents Solaris sh from translating var values in `set'!
        Non-C LC_CTYPE values break the ctype check.
    '''
    os.environ['LANG'] = os.getenv('LANG', 'C')
    os.environ['LC'] = os.getenv('LC_ALL', 'C')
    os.environ['LC_MESSAGE'] = os.getenv('LC_MESSAGE', 'C')
    os.environ['LC_CTYPE'] = os.getenv('LC_CTYPE', 'C')


def createDirectories():
    ''' Create the build directories if necessary '''
    for dir in ['bind', 'clipart', 'doc', 'examples', 'images', 'kbd', \
        'layouts', 'scripts', 'templates', 'ui' ]:
        if not os.path.isdir( dir ):
            try:
                os.mkdir( dir)
            except:
                print "Failed to create directory ", dir
                sys.exit(1)


def checkTeXPaths():
    ''' Determine the path-style needed by the TeX engine on Win32 (Cygwin) '''
    windows_style_tex_paths = ''
    if sys.platform == 'cygwin':
        from tempfile import mkstemp
        fd, tmpfname = mkstemp(suffix='.ltx', dir='/tmp')
        os.write(fd, r'\relax')
        os.close(fd)
        inpname = cmdOutput('cygpath -m ' + tmpfname)
        latex_out = cmdOutput(r'latex "\nonstopmode\input{%s}"' % inpname)
        if latex_out.find('Error') != -1:
            print "configure: TeX engine needs posix-style paths in latex files"
            windows_style_tex_paths = 'false'
        else:
            print "configure: TeX engine needs windows-style paths in latex files"
            windows_style_tex_paths = 'true'
        removeFiles([tmpfname, 'texput.log'])
    return windows_style_tex_paths


## Searching some useful programs
def checkProg(description, progs, rc_entry = [], path = [], not_found = ''):
    '''
        This function will search a program in $PATH plus given path
        If found, return directory and program name (not the options).

        description: description of the program

        progs: check programs, for each prog, the first word is used
            for searching but the whole string is used to replace
            %% for a rc_entry. So, feel free to add '$$i' etc for programs.

        path: additional pathes

        rc_entry: entry to outfile, can be
            1. emtpy: no rc entry will be added
            2. one pattern: %% will be replaced by the first found program,
                or '' if no program is found.
            3. several patterns for each prog and not_found. This is used 
                when different programs have different usages. If you do not 
                want not_found entry to be added to the RC file, you can specify 
                an entry for each prog and use '' for the not_found entry.

        not_found: the value that should be used instead of '' if no program
            was found

    '''
    # one rc entry for each progs plus not_found entry
    if len(rc_entry) > 1 and len(rc_entry) != len(progs) + 1:
        print "rc entry should have one item or item for each prog and not_found."
        sys.exit(2)
    print 'checking for ' + description + '...'
    ## print '(' + ','.join(progs) + ')',
    for idx in range(len(progs)):
        # ac_prog may have options, ac_word is the command name
        ac_prog = progs[idx]
        ac_word = ac_prog.split(' ')[0]
        print '+checking for "' + ac_word + '"... ',
        path = os.environ["PATH"].split(os.pathsep) + path
        for ac_dir in path:
            # check both ac_word and ac_word.exe (for windows system)
            if os.path.isfile( os.path.join(ac_dir, ac_word) ) or \
                os.path.isfile( os.path.join(ac_dir, ac_word + ".exe") ):
                print ' yes'
                # write rc entries for this command
                if len(rc_entry) == 1:
                    addToRC(rc_entry[0].replace('%%', ac_prog))
                elif len(rc_entry) > 1:
                    addToRC(rc_entry[idx].replace('%%', ac_prog))
                return [ac_dir, ac_word]
        # if not successful
        print ' no'
    # write rc entries for 'not found'
    if len(rc_entry) > 0:  # the last one.
        addToRC(rc_entry[-1].replace('%%', not_found))
    return ['', not_found]


def checkViewer(description, progs, rc_entry = [], path = []):
    ''' The same as checkProg, but for viewers and editors '''
    return checkProg(description, progs, rc_entry, path, not_found = 'auto')


def checkDTLtools():
    ''' Check whether DTL tools are available (Windows only) '''
    # Find programs! Returned path is not used now
    if ((os.name == 'nt' or sys.platform == 'cygwin') and
            checkProg('DVI to DTL converter', ['dv2dt']) != ['', ''] and
            checkProg('DTL to DVI converter', ['dt2dv']) != ['', '']):
        dtl_tools = True
    else:
        dtl_tools = False
    return dtl_tools


def checkLatex(dtl_tools):
    ''' Check latex, return lyx_check_config '''
    if dtl_tools:
        # Windows only: DraftDVI
        converter_entry = r'''\converter latex      dvi2       "%%"	"latex"
\converter dvi2       dvi        "python -tt $$s/scripts/clean_dvi.py $$i $$o"	""'''
    else:
        converter_entry = r'\converter latex      dvi        "%%"	"latex"'
    path, LATEX = checkProg('a Latex2e program', ['pplatex $$i', 'latex $$i', 'latex2e $$i'],
        rc_entry = [converter_entry])
    # no latex
    if LATEX != '':
        # Check if latex is usable
        writeToFile('chklatex.ltx', '''
\\nonstopmode\\makeatletter
\\ifx\\undefined\\documentclass\\else
    \\message{ThisIsLaTeX2e}
\\fi
\\@@end
''')
        # run latex on chklatex.ltx and check result
        if cmdOutput(LATEX + ' chklatex.ltx').find('ThisIsLaTeX2e') != -1:
            # valid latex2e
            return LATEX
        else:
            print "Latex not usable (not LaTeX2e) "
        # remove temporary files
        removeFiles(['chklatex.ltx', 'chklatex.log'])
    return ''


def checkFormatEntries(dtl_tools):  
    ''' Check all formats (\Format entries) '''
    checkViewer('a Tgif viewer and editor', ['tgif'],
        rc_entry = [r'\Format tgif       obj     Tgif                   "" "%%"	"%%"	"vector"'])
    #
    checkViewer('a FIG viewer and editor', ['xfig'],
        rc_entry = [r'\Format fig        fig     FIG                    "" "%%"	"%%"	"vector"'])
    #
    checkViewer('a Grace viewer and editor', ['xmgrace'],
        rc_entry = [r'\Format agr        agr     Grace                  "" "%%"	"%%"	"vector"'])
    #
    checkViewer('a FEN viewer and editor', ['xboard -lpf $$i -mode EditPosition'],
        rc_entry = [r'\Format fen        fen     FEN                    "" "%%"	"%%"'])
    #
    path, iv = checkViewer('a raster image viewer', ['xv', 'kview', 'gimp'])
    path, ie = checkViewer('a raster image editor', ['gimp'])
    addToRC(r'''\Format bmp        bmp     BMP                    "" "%s"	"%s"
\Format gif        gif     GIF                    "" "%s"	"%s"
\Format jpg        jpg     JPEG                   "" "%s"	"%s"
\Format pbm        pbm     PBM                    "" "%s"	"%s"
\Format pgm        pgm     PGM                    "" "%s"	"%s"
\Format png        png     PNG                    "" "%s"	"%s"
\Format ppm        ppm     PPM                    "" "%s"	"%s"
\Format tiff       tif     TIFF                   "" "%s"	"%s"
\Format xbm        xbm     XBM                    "" "%s"	"%s"
\Format xpm        xpm     XPM                    "" "%s"	"%s"''' % \
        (iv, ie, iv, ie, iv, ie, iv, ie, iv, ie, iv, ie, iv, ie, iv, ie, iv, ie, iv, ie) )
    #
    checkViewer('a text editor', ['xemacs', 'gvim', 'kedit', 'kwrite', 'kate', \
        'nedit', 'gedit', 'notepad'],
        rc_entry = [r'''\Format asciichess asc    "Plain text (chess output)"  "" ""	"%%"
\Format asciiimage asc    "Plain text (image)"         "" ""	"%%"
\Format asciixfig  asc    "Plain text (Xfig output)"   "" ""	"%%"
\Format dateout    tmp    "date (output)"         "" ""	"%%"
\Format docbook    sgml    DocBook                B  ""	"%%"
\Format docbook-xml xml   "Docbook (XML)"         "" ""	"%%"
\Format lilypond   ly     "LilyPond music"        "" ""	"%%"
\Format literate   nw      NoWeb                  N  ""	"%%"
\Format latex      tex    "LaTeX (plain)"         L  ""	"%%"
\Format linuxdoc   sgml    LinuxDoc               x  ""	"%%"
\Format pdflatex   tex    "LaTeX (pdflatex)"      "" ""	"%%"
\Format text       txt    "Plain text"            a  ""	"%%"
\Format textparagraph txt "Plain text (paragraphs)"    "" ""	"%%"''' ])
    #
    #checkProg('a Postscript interpreter', ['gs'],
    #  rc_entry = [ r'\ps_command "%%"' ])
    checkViewer('a Postscript previewer', ['gv', 'ghostview -swap', 'kghostview'],
        rc_entry = [r'''\Format eps        eps     EPS                    "" "%%"	""	"vector"
\Format ps         ps      Postscript             t  "%%"	""	"vector"'''])
    #
    checkViewer('a PDF previewer', ['acrobat', 'acroread', 'gv', 'ghostview', \
                            'xpdf', 'kpdf', 'kghostview'],
        rc_entry = [r'''\Format pdf        pdf    "PDF (ps2pdf)"          P  "%%"	""	"vector"
\Format pdf2       pdf    "PDF (pdflatex)"        F  "%%"	""	"vector"
\Format pdf3       pdf    "PDF (dvipdfm)"         m  "%%"	""	"vector"'''])
    #
    checkViewer('a DVI previewer', ['xdvi', 'kdvi'],
        rc_entry = [r'\Format dvi        dvi     DVI                    D  "%%"	""	"vector"'])
    if dtl_tools:
        # Windows only: DraftDVI
        addToRC(r'\Format dvi2       dvi     DraftDVI               ""	""	"vector"')
    #
    checkViewer('a HTML previewer', ['mozilla file://$$p$$i', 'netscape'],
        rc_entry = [r'\Format html       html    HTML                   H  "%%"	""'])
    #
    # entried that do not need checkProg
    addToRC(r'''\Format date       ""     "date command"          "" ""	""	""
\Format fax        ""      Fax                    "" ""	""	""
\Format lyx        lyx     LyX                    "" ""	""	""
\Format lyx13x     lyx13  "LyX 1.3.x"             "" ""	""	""
\Format lyxpreview lyxpreview "LyX Preview"       "" ""	""	""
\Format pdftex     pdftex_t PDFTEX                "" ""	""	""
\Format program    ""      Program                "" ""	""	""
\Format pstex      pstex_t PSTEX                  "" ""	""	""
\Format rtf        rtf    "Rich Text Format"      "" ""	""	"vector"
\Format sxw        sxw    "OpenOffice.Org Writer" O  ""	""	"vector"
\Format wmf        wmf    "Windows Metafile"      "" ""	""	"vector"
\Format emf        emf    "Enhanced Metafile"     "" ""	""	"vector"
\Format word       doc    "MS Word"               W  ""	""	"vector"
\Format wordhtml   html   "MS Word (HTML)"        "" ""        ""	""
''')


def checkConverterEntries():
    ''' Check all converters (\converter entries) '''
    checkProg('the pdflatex program', ['pdflatex $$i'],
        rc_entry = [ r'\converter pdflatex   pdf2       "%%"	"latex"' ])
    
    ''' If we're running LyX in-place then tex2lyx will be found in
            ../src/tex2lyx. Add this directory to the PATH temporarily and
            search for tex2lyx.
            Use PATH to avoid any problems with paths-with-spaces.
    '''
    path_orig = os.environ["PATH"]
    os.environ["PATH"] = os.path.join('..', 'src', 'tex2lyx') + \
        os.pathsep + path_orig

    checkProg('a LaTeX/Noweb -> LyX converter', ['tex2lyx', 'tex2lyx' + version_suffix],
        rc_entry = [r'''\converter latex      lyx        "%% -f $$i $$o"	""
\converter literate   lyx        "%% -n -f $$i $$o"	""'''])

    os.environ["PATH"] = path_orig

    #
    checkProg('a Noweb -> LaTeX converter', ['noweave -delay -index $$i > $$o'],
        rc_entry = [ r'\converter literate   latex      "%%"	""' ])
    #
    checkProg('a HTML -> LaTeX converter', ['html2latex $$i'],
        rc_entry = [ r'\converter html       latex      "%%"	""' ])
    #
    checkProg('a MSWord -> LaTeX converter', ['wvCleanLatex $$i $$o'],
        rc_entry = [ r'\converter word       latex      "%%"	""' ])
    #
    checkProg('a LaTeX -> MS Word converter', ["htlatex $$i 'html,word' 'symbol/!' '-cvalidate'"],
        rc_entry = [ r'\converter latex      wordhtml   "%%"	""' ])
    #
    checkProg('an OpenOffice.org -> LaTeX converter', ['w2l -clean $$i'],
        rc_entry = [ r'\converter sxw        latex      "%%"	""' ])
    #
    checkProg('a LaTeX -> OpenOffice.org converter', ['oolatex $$i', 'oolatex.sh $$i'],
        rc_entry = [ r'\converter latex      sxw        "%%"	"latex"' ])
    # On windows it is called latex2rt.exe
    checkProg('a LaTeX -> RTF converter', ['latex2rtf -p -S -o $$o $$i', 'latex2rt -p -S -o $$o $$i'],
        rc_entry = [ r'\converter latex      rtf        "%%"	"needaux"' ])
    #
    checkProg('a PS to PDF converter', ['ps2pdf13 $$i $$o'],
        rc_entry = [ r'\converter ps         pdf        "%%"	""' ])
    #
    checkProg('a DVI to PS converter', ['dvips -o $$o $$i'],
        rc_entry = [ r'\converter dvi        ps         "%%"	""' ])
    #
    checkProg('a DVI to PDF converter', ['dvipdfmx -o $$o $$i', 'dvipdfm -o $$o $$i'],
        rc_entry = [ r'\converter dvi        pdf3       "%%"	""' ])
    #
    path, dvipng = checkProg('dvipng', ['dvipng'])
    if dvipng == "dvipng":
        addToRC(r'\converter lyxpreview png        "python -tt $$s/scripts/lyxpreview2bitmap.py"	""')
    else:
        addToRC(r'\converter lyxpreview png        ""	""')
    #  
    checkProg('a fax program', ['kdeprintfax $$i', 'ksendfax $$i'],
        rc_entry = [ r'\converter ps         fax        "%%"	""'])
    #
    checkProg('a FIG -> EPS/PPM converter', ['fig2dev'],
        rc_entry = [
            r'''\converter fig        eps        "fig2dev -L eps $$i $$o"	""
\converter fig        ppm        "fig2dev -L ppm $$i $$o"	""
\converter fig        png        "fig2dev -L png $$i $$o"	""''',
            ''])
    #
    checkProg('a TIFF -> PS converter', ['tiff2ps $$i > $$o'],
        rc_entry = [ r'\converter tiff       eps        "%%"	""', ''])
    #
    checkProg('a TGIF -> EPS/PPM converter', ['tgif'],
        rc_entry = [
            r'''\converter tgif       eps        "tgif -stdout -print -color -eps $$i > $$o"	""
\converter tgif       ppm        "tgif -stdout -print -color -ppm $$i > $$o"	""
\converter tgif       png        "tgif -stdout -print -color -png $$i > $$o"	""
\converter tgif       pdf        "tgif -stdout -print -color -pdf $$i > $$o"	""''',
            ''])
    #
    checkProg('a WMF -> EPS converter', ['metafile2eps $$i $$o', 'wmf2eps -o $$o $$i'],
        rc_entry = [ r'\converter wmf        eps        "%%"	""'])
    #
    checkProg('an EMF -> EPS converter', ['metafile2eps $$i $$o', 'wmf2eps -o $$o $$i'],
        rc_entry = [ r'\converter emf        eps        "%%"	""'])
    #
    checkProg('a EPS -> PDF converter', ['epstopdf'],
        rc_entry = [ r'\converter eps        pdf        "epstopdf --outfile=$$o $$i"	""', ''])
    #
    checkProg('a Grace -> Image converter', ['gracebat'],
        rc_entry = [
            r'''\converter agr        eps        "gracebat -hardcopy -printfile $$o -hdevice EPS $$i 2>/dev/null"	""
\converter agr        png        "gracebat -hardcopy -printfile $$o -hdevice PNG $$i 2>/dev/null"	""
\converter agr        jpg        "gracebat -hardcopy -printfile $$o -hdevice JPEG $$i 2>/dev/null"	""
\converter agr        ppm        "gracebat -hardcopy -printfile $$o -hdevice PNM $$i 2>/dev/null"	""''',
            ''])
    #
    checkProg('a LaTeX -> HTML converter', ['htlatex $$i', 'tth  -t -e2 -L$$b < $$i > $$o', \
        'latex2html -no_subdir -split 0 -show_section_numbers $$i', 'hevea -s $$i'],
        rc_entry = [ r'\converter latex      html       "%%"	"originaldir,needaux"' ])
    #
    path, lilypond = checkProg('a LilyPond -> ESP/PDF/PNG converter', ['lilypond'])
    if (lilypond != ''):
        version_string = cmdOutput("lilypond --version")
        match = re.match('GNU LilyPond (\S+)', version_string)
        if match:
            version_number = match.groups()[0]
            version = version_number.split('.')
            if int(version[0]) > 2 or (len(version) > 1 and int(version[0]) == 2 and int(version[1]) >= 6):
                addToRC(r'''\converter lilypond   eps        "lilypond -b eps --ps $$i"	""
\converter lilypond   png        "lilypond -b eps --png $$i"	""''')
                if int(version[0]) > 2 or (len(version) > 1 and int(version[0]) == 2 and int(version[1]) >= 9):
                    addToRC(r'\converter lilypond   pdf        "lilypond -b eps --pdf $$i"	""')
                print '+  found LilyPond version %s.' % version_number
            else:
                print '+  found LilyPond, but version %s is too old.' % version_number
        else:
            print '+  found LilyPond, but could not extract version number.'
    #
    # FIXME: no rc_entry? comment it out
    # checkProg('Image converter', ['convert $$i $$o'])
    #
    # Entried that do not need checkProg
    addToRC(r'''\converter lyxpreview ppm        "python -tt $$s/scripts/lyxpreview2bitmap.py"	""
\converter date       dateout    "python -tt $$s/scripts/date.py %d-%m-%Y > $$o"	""
\converter docbook    docbook-xml "cp $$i $$o"	"xml"
\converter fen        asciichess "python -tt $$s/scripts/fen2ascii.py $$i $$o"	""
\converter fig        pdftex     "python -tt $$s/scripts/fig2pdftex.py $$i $$o"	""
\converter fig        pstex      "python -tt $$s/scripts/fig2pstex.py $$i $$o"	""
\converter lyx        lyx13x     "python -tt $$s/lyx2lyx/lyx2lyx -t 221 $$i > $$o"	""
''')


def checkLinuxDoc():
    ''' Check linuxdoc '''
    #
    path, LINUXDOC = checkProg('SGML-tools 1.x (LinuxDoc)', ['sgml2lyx'],
        rc_entry = [
        r'''\converter linuxdoc   lyx        "sgml2lyx $$i"	""
\converter linuxdoc   latex      "sgml2latex $$i"	""
\converter linuxdoc   dvi        "sgml2latex -o dvi $$i"	""
\converter linuxdoc   html       "sgml2html $$i"	""''',
        r'''\converter linuxdoc   lyx        ""	""
\converter linuxdoc   latex      ""	""
\converter linuxdoc   dvi        ""	""
\converter linuxdoc   html       ""	""''' ])
    if LINUXDOC != '':
        return ('yes', 'true', '\\def\\haslinuxdoc{yes}')
    else:
        return ('no', 'false', '')


def checkDocBook():
    ''' Check docbook '''
    path, DOCBOOK = checkProg('SGML-tools 2.x (DocBook) or db2x scripts', ['sgmltools', 'db2dvi'],
        rc_entry = [
            r'''\converter docbook    dvi        "sgmltools -b dvi $$i"	""
\converter docbook    html       "sgmltools -b html $$i"	""''',
            r'''\converter docbook    dvi        "db2dvi $$i"	""
\converter docbook    html       "db2html $$i"	""''',
            r'''\converter docbook    dvi        ""	""
\converter docbook    html       ""	""'''])
    #
    if DOCBOOK != '':
        return ('yes', 'true', '\\def\\hasdocbook{yes}')
    else:
        return ('no', 'false', '')


def checkOtherEntries():
    ''' entries other than Format and Converter '''
    checkProg('a *roff formatter', ['groff', 'nroff'],
        rc_entry = [
            r'\ascii_roff_command "groff -t -Tlatin1 $$FName"',
            r'\ascii_roff_command "tbl $$FName | nroff"',
            r'\ascii_roff_command ""' ])
    checkProg('ChkTeX', ['chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38'],
        rc_entry = [ r'\chktex_command "%%"' ])
    checkProg('a spellchecker', ['ispell'],
        rc_entry = [ r'\spell_command "%%"' ])
    ## FIXME: OCTAVE is not used anywhere
    # path, OCTAVE = checkProg('Octave', ['octave'])
    ## FIXME: MAPLE is not used anywhere
    # path, MAPLE = checkProg('Maple', ['maple'])
    checkProg('a spool command', ['lp', 'lpr'],
        rc_entry = [
            r'''\print_spool_printerprefix "-d "
\print_spool_command "lp"''',
            r'''\print_spool_printerprefix "-P",
\print_spool_command "lpr"''',
            ''])
    # Add the rest of the entries (no checkProg is required)
    addToRC(r'''\copier    fig        "python -tt $$s/scripts/fig_copy.py $$i $$o"
\copier    pstex      "python -tt $$s/scripts/tex_copy.py $$i $$o $$l"
\copier    pdftex     "python -tt $$s/scripts/tex_copy.py $$i $$o $$l"
''')


def processLayoutFile(file, bool_docbook, bool_linuxdoc):
    ''' process layout file and get a line of result
        
        Declear line are like this: (article.layout, scrbook.layout, svjog.layout)
        
        \DeclareLaTeXClass{article}
        \DeclareLaTeXClass[scrbook]{book (koma-script)}
        \DeclareLaTeXClass[svjour,svjog.clo]{article (Springer - svjour/jog)}

        we expect output:
        
        "article" "article" "article" "false"
        "scrbook" "scrbook" "book (koma-script)" "false"
        "svjog" "svjour" "article (Springer - svjour/jog)" "false"
    '''
    classname = file.split(os.sep)[-1].split('.')[0]
    # return ('LaTeX', '[a,b]', 'a', ',b,c', 'article') for \DeclearLaTeXClass[a,b,c]{article}
    p = re.compile(r'\Declare(LaTeX|DocBook|LinuxDoc)Class\s*(\[([^,]*)(,.*)*\])*\s*{(.*)}')
    for line in open(file).readlines():
        res = p.search(line)
        if res != None:
            (classtype, optAll, opt, opt1, desc) = res.groups()
            avai = {'LaTeX':'false', 'DocBook':bool_docbook, 'LinuxDoc':bool_linuxdoc}[classtype]
            if opt == None:
                opt = classname
            return '"%s" "%s" "%s" "%s"\n' % (classname, opt, desc, avai)
    print "Layout file without \DeclearXXClass line. "
    sys.exit(2)

    
def checkLatexConfig(check_config, bool_docbook, bool_linuxdoc):
    ''' Explore the LaTeX configuration '''
    print 'checking LaTeX configuration... ',
    # First, remove the files that we want to re-create
    removeFiles(['textclass.lst', 'packages.lst', 'chkconfig.sed'])
    #
    if not check_config:
        print ' default values'
        print '+checking list of textclasses... '
        tx = open('textclass.lst', 'w')
        tx.write('''
# This file declares layouts and their associated definition files
# (include dir. relative to the place where this file is).
# It contains only default values, since chkconfig.ltx could not be run
# for some reason. Run ./configure.py if you need to update it after a
# configuration change.
''')
        # build the list of available layout files and convert it to commands
        # for chkconfig.ltx
        foundClasses = []
        for file in glob.glob( os.path.join('layouts', '*.layout') ) + \
            glob.glob( os.path.join(srcdir, 'layouts', '*.layout' ) ) :
            # valid file?
            if not os.path.isfile(file): 
                continue
            # get stuff between /xxxx.layout .
            classname = file.split(os.sep)[-1].split('.')[0]
            #  tr ' -' '__'`
            cleanclass = classname.replace(' ', '_')
            cleanclass = cleanclass.replace('-', '_')
            # make sure the same class is not considered twice
            if foundClasses.count(cleanclass) == 0: # not found before
                foundClasses.append(cleanclass)
                tx.write(processLayoutFile(file, bool_docbook, bool_linuxdoc))
        tx.close()
        print '\tdone'
    else:
        print '\tauto'
        removeFiles(['wrap_chkconfig.ltx', 'chkconfig.vars', \
            'chkconfig.classes', 'chklayouts.tex'])
        rmcopy = False
        if not os.path.isfile( 'chkconfig.ltx' ):
            shutil.copy( os.path.join(srcdir, 'chkconfig.ltx'),  'chkconfig.ltx' )
            rmcopy = True
        writeToFile('wrap_chkconfig.ltx', '%s\n%s\n\\input{chkconfig.ltx}\n' \
            % (linuxdoc_cmd, docbook_cmd) )
        # Construct the list of classes to test for.
        # build the list of available layout files and convert it to commands
        # for chkconfig.ltx
        p1 = re.compile(r'\Declare(LaTeX|DocBook|LinuxDoc)Class')
        testclasses = list()
        for file in glob.glob( os.path.join('layouts', '*.layout') ) + \
            glob.glob( os.path.join(srcdir, 'layouts', '*.layout' ) ) :
            if not os.path.isfile(file):
                continue
            classname = file.split(os.sep)[-1].split('.')[0]
            for line in open(file).readlines():
                if p1.search(line) == None:
                    continue
                if line[0] != '#':
                    print "Wrong input layout file with line '" + line
                    sys.exit(3)
                testclasses.append("\\TestDocClass{%s}{%s}" % (classname, line[1:].strip()))
                break
        testclasses.sort()
        cl = open('chklayouts.tex', 'w')
        for line in testclasses:
            cl.write(line + '\n')
        cl.close()
        #
        # we have chklayouts.tex, then process it
        fout = os.popen(LATEX + ' wrap_chkconfig.ltx')
        while True:
            line = fout.readline()
            if not line:
                break;
            if re.match('^\+', line):
                print line,
        fout.close()
        #
        # currently, values in chhkconfig are only used to set
        # \font_encoding
        values = {}
        for line in open('chkconfig.vars').readlines():
            key, val = re.sub('-', '_', line).split('=')
            val = val.strip()
            try:
                values[key] = val.strip("'")
            except TypeError:
                # workaround for python 2.2.0 and 2.2.1
                tmp = val.split("'")
                while tmp and not tmp[0]: tmp = tmp[1:]
                while tmp and not tmp[-1]: tmp = tmp[:-1]
                values[key] = "'".join(tmp)
        # chk_fontenc may not exist 
        try:
            addToRC(r'\font_encoding "%s"' % values["chk_fontenc"])
        except:
            pass
        if rmcopy:   # remove the copied file
            removeFiles( [ 'chkconfig.ltx' ] )


def createLaTeXConfig():
    ''' create LaTeXConfig.lyx '''
    # if chkconfig.sed does not exist (because LaTeX did not run),
    # then provide a standard version.
    if not os.path.isfile('chkconfig.sed'):
        writeToFile('chkconfig.sed', 's!@.*@!???!g\n')
    print "creating packages.lst"
    # if packages.lst does not exist (because LaTeX did not run),
    # then provide a standard version.
    if not os.path.isfile('packages.lst'):
        writeToFile('packages.lst', '''
### This file should contain the list of LaTeX packages that have been
### recognized by LyX. Unfortunately, since configure could not find
### your LaTeX2e program, the tests have not been run. Run ./configure.py
### if you need to update it after a configuration change.
''')
    print 'creating doc/LaTeXConfig.lyx'
    #
    # This is originally done by sed, using a
    # tex-generated file chkconfig.sed
    ##sed -f chkconfig.sed ${srcdir}/doc/LaTeXConfig.lyx.in
    ##  >doc/LaTeXConfig.lyx
    # Now, we have to do it by hand (python).
    #
    # add to chekconfig.sed
    writeToFile('chkconfig.sed', '''s!@chk_linuxdoc@!%s!g
s!@chk_docbook@!%s!g
    ''' % (chk_linuxdoc, chk_docbook) , append=True)
    # process this sed file!!!!
    lyxin = open( os.path.join(srcdir, 'doc', 'LaTeXConfig.lyx.in')).readlines()
    # get the rules
    p = re.compile(r's!(.*)!(.*)!g')
    # process each sed replace.
    for sed in open('chkconfig.sed').readlines():
        if sed.strip() == '':
            continue
        try:
            fr, to = p.match(sed).groups()
            # if latex did not run, change all @name@ to '???'
            if fr == '@.*@':
                for line in range(len(lyxin)):
                    lyxin[line] = re.sub('@.*@', to, lyxin[line])
            else:
                for line in range(len(lyxin)):
                    lyxin[line] = lyxin[line].replace(fr, to)
        except:  # wrong sed entry?
            print "Wrong sed entry in chkconfig.sed: '" + sed + "'"
            sys.exit(4)
    # 
    writeToFile( os.path.join('doc', 'LaTeXConfig.lyx'),
        ''.join(lyxin))


def checkTeXAllowSpaces():
    ''' Let's check whether spaces are allowed in TeX file names '''
    tex_allows_spaces = 'false'
    if lyx_check_config:
        print "Checking whether TeX allows spaces in file names... ",
        writeToFile('a b.tex', r'\message{working^^J}' )
        if os.name == 'nt':
            latex_out = cmdOutput(LATEX + r""" "\nonstopmode\input{\"a b\"}" """)
        else:
            latex_out = cmdOutput(LATEX + r""" '\nonstopmode\input{"a b"}' """)
        if latex_out.find('working') != -1:
            print 'yes'
            tex_allows_spaces = 'true'
        else:
            print 'no'
            tex_allows_spaces = 'false'
        addToRC(r'\tex_allows_spaces ' + tex_allows_spaces)
        removeFiles( [ 'a b.tex', 'a b.log', 'texput.log' ])


def removeTempFiles():
    # Final clean-up
    if not lyx_keep_temps:
        removeFiles(['chkconfig.sed', 'chkconfig.vars',  \
            'wrap_chkconfig.ltx', 'wrap_chkconfig.log', \
            'chklayouts.tex', 'missfont.log', 
            'chklatex.ltx', 'chklatex.log'])


if __name__ == '__main__':
    lyx_check_config = True
    outfile = 'lyxrc.defaults'
    rc_entries = ''
    lyx_keep_temps = False
    version_suffix = ''
    logfile = 'configure.log'
    ## Parse the command line
    for op in sys.argv[1:]:   # default shell/for list is $*, the options
        if op in [ '-help', '--help', '-h' ]:
            print '''Usage: configure [options]
Options:
    --help                   show this help lines
    --keep-temps             keep temporary files (for debug. purposes)
    --without-latex-config   do not run LaTeX to determine configuration
    --with-version-suffix=suffix suffix of binary installed files
'''
            sys.exit(0)
        elif op == '--without-latex-config':
            lyx_check_config = False
        elif op == '--keep-temps':
            lyx_keep_temps = True
        elif op[0:22] == '--with-version-suffix=':  # never mind if op is not long enough
            version_suffix = op[22:]
        else:
            print "Unknown option", op
            sys.exit(1)
    #
    # set up log file for stdout and stderr
    log = open(logfile, 'w')
    sys.stdout = Tee(sys.stdout, log)
    sys.stderr = Tee(sys.stderr, log)
    # check if we run from the right directory
    srcdir = os.path.dirname(sys.argv[0])
    if srcdir == '':
        srcdir = '.'
    if not os.path.isfile( os.path.join(srcdir, 'chkconfig.ltx') ):
        print "configure: error: cannot find chkconfig.ltx script"
        sys.exit(1)
    setEnviron()
    createDirectories()
    windows_style_tex_paths = checkTeXPaths()
    dtl_tools = checkDTLtools()
    ## Write the first part of outfile
    writeToFile(outfile, '''# This file has been automatically generated by LyX' lib/configure.py
# script. It contains default settings that have been determined by
# examining your system. PLEASE DO NOT MODIFY ANYTHING HERE! If you
# want to customize LyX, use LyX' Preferences dialog or modify directly 
# the "preferences" file instead. Any setting in that file will
# override the values given here.
''')
    # check latex
    LATEX = checkLatex(dtl_tools)
    checkFormatEntries(dtl_tools)
    checkConverterEntries()
    (chk_linuxdoc, bool_linuxdoc, linuxdoc_cmd) = checkLinuxDoc()
    (chk_docbook, bool_docbook, docbook_cmd) = checkDocBook()
    checkTeXAllowSpaces()
    if windows_style_tex_paths != '':
        addToRC(r'\tex_expects_windows_paths %s' % windows_style_tex_paths)
    checkOtherEntries()
    # --without-latex-config can disable lyx_check_config
    checkLatexConfig( lyx_check_config and LATEX != '', bool_docbook, bool_linuxdoc)
    createLaTeXConfig()
    removeTempFiles()
