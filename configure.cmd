/* OS/2 REXX : The top level configure.cmd; Don't delete this comment. */
call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
call SysLoadFuncs
intl='yes'
latexconfig='yes'
extraopt=''
static='no'
optionsread=0
parse arg option1 option2 option3 gabbages
if (option1='--disable-nls')|(option2='--disable-nls')|(option3='--disable-nls') then
  do
    optionsread=optionsread+1
    intl='no'
  end
if (option1='--with-extra-optimize')|(option2='--with-extra-optimize')|(option3='--with-extra-optimize') then
  do
    optionsread=optionsread+1
    '@gcc -v 2>&1|grep 2.7'
    if rc=0 then
      extraopt='-malign-loops=2 -malign-jumps=2 -malign-functions=2 -ffast-math'
  end
if (option1='--link-static-xforms')|(option2='--link-static-xforms')|(option3='--link-static-xforms') then
  do
    optionsread=optionsread+1
    static='yes'
  end
if \(gabbages='')|(\(option3='')&(optionsread<3))|(\(option2='')&(optionsread<2))|(\(option1='')&(optionsread<1)) then
  do
    say 'Unknown option! Use only: --with-extra-optimize --disable-nls --link-static-xforms'
    exit
  end

env='OS2ENVIRONMENT'
X11ROOT=value('X11ROOT',,env)

/* I don't think I know how to substitute all the occurences    *
 * of a substring to something else with REXX comes with Warp3. *
 * Of course Object REXX has ANSI function changestr            */
curdir=directory()
do until pos('\',curdir)=0
  parse var curdir tmp1 '\' tmp2
  curdir=tmp1'#/'tmp2
end
curdir=translate(curdir,'\','#')

os2config.1='@prefix@ 'X11ROOT'\/XFree86'
os2config.2='@exec_prefix@ 'subword(os2config.1,2)
os2config.3='@srcdir@ 'curdir
os2config.4='@top_srcdir@ 'subword(os2config.3,2)
os2config.5='@bindir@ 'subword(os2config.2,2)'\/bin'
os2config.6='@libdir@ 'subword(os2config.2,2)'\/lib'
os2config.7='@datadir@ 'subword(os2config.2,2)'\/lib'
os2config.8='@mandir@ 'subword(os2config.1,2)'\/man'
os2config.9='@CPPIMPLFLAGS@ -I'subword(os2config.1,2)'\/include'
os2config.10='@program_suffix@'
os2config.11='@lyx_devel_version@ $(lyx_devel_version)'
os2config.12='@PACKAGE@ lyx'
os2config.13='@DATADIRNAME@ lib'

contains=0
do until contains > 0
  testversion=linein('src/version.h')
  contains=pos('#define LYX_VERSION', testversion)
end
os2config.14='@LYX_VERSION@ 'strip(subword(testversion,3,1),,'"')
os2config.15='@VERSION@ 'subword(os2config.14,2)

CXXFLAGS=value('CXXFLAGS',,env)
if CXXFLAGS='' then
  do
    CXXFLAGS='-g -m486 -O -fno-strength-reduce -Zmtd -Zsysv-signals -D__ST_MT_ERRNO__ -Wall -Wno-unused'
    '@gcc -v 2>&1|grep 2.8'
    if rc=0 then
      CXXFLAGS=CXXFLAGS' -fno-exceptions -fno-rtti'
  end
CFLAGS=value('CFLAGS',,env)
if CFLAGS='' then
  CFLAGS='-g -m486 -O2 -fno-strength-reduce -Zmtd -Zsysv-signals -D__ST_MT_ERRNO__ -Wall -Wno-unused'
/* extraopt is added to CXXFLAGS/CFLAGS even if the environment variable is set */
CXXFLAGS=CXXFLAGS' 'extraopt
CFLAGS=CFLAGS' 'extraopt
os2config.16='@CXXFLAGS@ 'CXXFLAGS
os2config.17='@CFLAGS@ 'CFLAGS

os2config.18='@CC@ gcc'
os2config.19='@CXX@ gcc'   /* os2config.12='@CXX@ g++' */
os2config.20='@RANLIB@ ar s'
if static='yes' then
  os2config.21='@LYX_LIBS@ -lforms_s -lulib -lXpm'
else
  os2config.21='@LYX_LIBS@ -lforms -lXpm'
os2config.22='@LIBS@ -lSM -lICE -lX11 -lsocket'

if opt='yes' then
  do
    os2config.23='@LDFLAGS@ -Zmtd -Zsysv-signals -Zbin-files -s -lwrap -L'subword(os2config.6,2)
  end
else
  do
    os2config.23='@LDFLAGS@ -Zmtd -Zsysv-signals -Zbin-files -L'subword(os2config.6,2)
  end

os2config.24='@CPPFLAGS@ -I'subword(os2config.1,2)'\/include -I..\/intl'
os2config.25='@SET_MAKE@'  /* ??? */
os2config.26='@CXX_COMMAND@ gcc.exe'

/* Full path is given in order to distinguish it from IBM install. */
os2config.27='@INSTALL@ 'subword(os2config.5,2)'\/install'
os2config.28='@INSTALL_DATA@ 'subword(os2config.5,2)'\/install'
os2config.29='@INSTALL_PROGRAM@ 'subword(os2config.5,2)'\/install'
os2config.30='@INSTALL_TARGET@ install-os2'
os2config.31='@LIBOBJS@'
if intl='yes' then
  do
    os2config.32='@INTLLIBS@ ..\/intl\/libintl.a'
    os2config.33='@INTLSUB@ intl'
    os2config.34='@POSUB@'	/* os2config.27='@POSUB@ po' */
    os2config.35='@USE_INCLUDED_LIBINTL@ yes'
  end
else
  do
    os2config.32='@INTLLIBS@'
    os2config.33='@INTLSUB@'
    os2config.34='@POSUB@'
    os2config.35='@USE_INCLUDED_LIBINTL@'
  end
os2config.36='@INTLOBJS@'
os2config.37='@DEFS@'
os2config.38='@program_transform_name@'
os2config.39='@l@'
os2config.40='@EXEEXT@ .exe'
os2config.41='@PERL@ perl'
os2config.42='@LYX_DIR@ $ENV{X11ROOT}\/XFree86\/lib\/X11\/lyx'
os2config.0=42

rc=SysFileDelete('os2config.sed')
do i=1 to os2config.0
  parse var os2config.i from_string to_string
  rc=lineout('os2config.sed','s/'from_string'/'to_string'/g')
end
if devel='no' then
  rc=lineout('os2config.sed','s/lyx_devel_version = yes/lyx_devel_version = no/')
rc=lineout('os2config.sed','s/#undef HAVE_ALLOCA$/#define HAVE_ALLOCA 1/')
rc=lineout('os2config.sed','s/#undef HAVE_ALLOCA_H/#define HAVE_ALLOCA_H 1/')
rc=lineout('os2config.sed','s/#undef STDC_HEADERS/#define STDC_HEADERS 1/')
rc=lineout('os2config.sed','s/#undef PACKAGE/#define PACKAGE \"lyx\"/')
rc=lineout('os2config.sed','s/#undef VERSION/#define VERSION \"'subword(os2config.14,2)'\"/')
rc=lineout('os2config.sed','s/#undef HAVE_GETCWD/#define HAVE_GETCWD 1/')
rc=lineout('os2config.sed','s/#undef HAVE_GETPAGESIZE/#define HAVE_GETPAGESIZE 1/')
rc=lineout('os2config.sed','s/#undef HAVE_MEMMOVE/#define HAVE_MEMMOVE 1/')
rc=lineout('os2config.sed','s/#undef HAVE_MEMSET/#define HAVE_MEMSET 1/')
rc=lineout('os2config.sed','s/#undef HAVE_PUTENV/#define HAVE_PUTENV 1/')
rc=lineout('os2config.sed','s/#undef HAVE_SETLOCALE/#define HAVE_SETLOCALE 1/')
rc=lineout('os2config.sed','s/#undef HAVE_XOPENIM/#define HAVE_XOPENIM 1/')
rc=lineout('os2config.sed','s/#undef HAVE_STRCHR/#define HAVE_STRCHR 1/')
rc=lineout('os2config.sed','s/#undef HAVE_STRERROR/#define HAVE_STRERROR 1/')
rc=lineout('os2config.sed','s/#undef HAVE_DIRENT_H/#define HAVE_DIRENT_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_ERRNO_H/#define HAVE_ERRNO_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_LIMITS_H/#define HAVE_LIMITS_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_LOCALE_H/#define HAVE_LOCALE_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_MALLOC_H/#define HAVE_MALLOC_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_STRING_H/#define HAVE_STRING_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_SYS_DIR_H/#define HAVE_SYS_DIR_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_SYS_SELECT_H/#define HAVE_SYS_SELECT_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_SYS_TIME_H/#define HAVE_SYS_TIME_H 1/')
rc=lineout('os2config.sed','s/#undef HAVE_UNISTD_H/#define HAVE_UNISTD_H 1/')
rc=lineout('os2config.sed','s/#undef XPM_H_LOCATION/#define XPM_H_LOCATION <X11\/xpm.h>/')
rc=lineout('os2config.sed','s/#undef FORMS_H_LOCATION/#define FORMS_H_LOCATION <forms.h>/')
rc=lineout('os2config.sed','s/#undef SIZEOF_INT/#define SIZEOF_INT 4/')
rc=lineout('os2config.sed','s/#undef SIZEOF_VOID_P/#define SIZEOF_VOID_P 4/')
rc=lineout('os2config.sed','s/#undef HAVE_bool/#define HAVE_bool 1/')
rc=lineout('os2config.sed','s/#undef SELECT_TYPE_ARG1/#define SELECT_TYPE_ARG1 (int)/')
rc=lineout('os2config.sed','s/#undef SELECT_TYPE_ARG234/#define SELECT_TYPE_ARG234 (fd_set *)/')
rc=lineout('os2config.sed','s/#undef SELECT_TYPE_ARG5/#define SELECT_TYPE_ARG5 (struct timeval *)/')
rc=lineout('os2config.sed','s/#undef WITH_WARNINGS/#define WITH_WARNINGS 1/')
if devel='yes' then
rc=lineout('os2config.sed','s/#undef DEVEL_VERSION/#define DEVEL_VERSION 1/')
rc=lineout('os2config.sed','s/SHELL/#SHELL/')
rc=lineout('os2config.sed','s/SCRIPT_SUFFIX=/SCRIPT_SUFFIX=.cmd/')
rc=lineout('os2config.sed','s/^DEFS = -DLOCALEDIR/DEFS = -DHAVE_CONFIG_H=1 -DLOCALEDIR/')
if intl='yes' then
rc=lineout('os2config.sed','s/#undef ENABLE_NLS/#define ENABLE_NLS 1/')
/*
rc=lineout('os2config.sed','s/  for [^ ]* in / -for %1 in (/')
rc=lineout('os2config.sed','s/; do /) do /')
*/
rc=lineout('os2config.sed','s/(cd lib && $(MAKE) install)/(cd lib && $(MAKE) os2install)/')
rc=lineout('os2config.sed','s/ln -s/cp/g')
rc=lineout('os2config.sed','s/-o lyx /-o lyx.exe /')
rc=lineout('os2config.sed','s/$(INSTALL_PROGRAM) reLyX\/reLyX $(bindir)\/reLyX/$(INSTALL_PROGRAM) reLyX\/reLyX.cmd $(bindir)\/reLyX.cmd/')
rc=lineout('os2config.sed','s/"reLyX /"reLyX.cmd /')
rc=lineout('os2config.sed')   /* Close the file */

/* Test if xforms and Xpm are available. */
rc=SysFileTree(X11ROOT'\XFree86\lib\forms.a',found,'F')
if found.0 = 0 then
  say "Can't find forms.a. Please check that the xforms library is correctly installed on your system."
rc=SysFileTree(X11ROOT'\XFree86\lib\Xpm.a',found,'F')
if found.0 = 0 then
  say "Can't find Xpm.a. Please check that the Xpm library is correctly installed on your system."

'touch config.status'
'touch config/stamp-h.in'
'sed -f os2config.sed Makefile.in > Makefile'
'sed -f os2config.sed lib/Makefile.in > lib\Makefile'
'sed -f os2config.sed src/Makefile.in > src\Makefile'
'sed -f os2config.sed src/config.h.in > src\config.h'
if intl='yes' then
  call SysFileTree 'intl\intlh.inst','patched','F'
  if patched.0 = 0 then
  /* Figure out how GNU patch program is called here */
  /* If run without arguments,
     GNU patch emits "usage" into stderr and none into stdout, and
     IBM patch emits nothing into stderr and an error message "SYS1575..." into stdout
  */
  do
    do
    '@patch 2>&1 | grep SYS1575 >nul'
    if rc=0 then
      do
        say 'configure tries to run GNU patch with the command "patch"'
        say 'and detects IBM patch instead.  Please type the collect'
        say 'name to invoke GNU patch:'
        parse pull patch
      end
    else patch='patch'
        patch' -d intl<development\OS2\gnugettext.diff'
        'touch intl/intlh.inst'
        'cp intl/libgettext.h intl/libintl.h'
    'sed -f os2config.sed intl/Makefile.in > intl\Makefile'
    end
  end
'cd lib'
/* Check if perl 5.002 or later is available */
PERL=SysSearchPath('PATH', 'perl.exe')
if PERL='' then PERL=SysSearchPath('PATH', 'perl_.exe')
if PERL='' then PERL=SysSearchPath('PATH', 'perl__.exe')
if PERL='' then PERL=SysSearchPath('PATH', 'perl___.exe')
PERL ' -e "require 5.002"'
if rc=0 then
  do
    call SysFileDelete 'reLyX\reLyX.cmd'
    rc=lineout('reLyX\reLyX.cmd','extproc 'filespec("name",PERL)' -S')
    rc=lineout('reLyX\reLyX.cmd')
    'sed -f ../os2config.sed reLyX/reLyX.in >> reLyX\reLyX.cmd'
    rc=SysFileTree('reLyX\reLyXmain.pl.in',found,'F')
    if found.0 = 0 then
      'cmd /c copy reLyX\reLyXmain.pl reLyX\reLyXmain.pl.in'
    'sed -e "s/noweb2lyx/noweb2lyx.cmd/" reLyX/reLyXmain.pl.in > reLyX\reLyXmain.pl'
    call SysFileDelete 'reLyX\noweb2lyx.cmd'
    rc=lineout('reLyX\noweb2lyx.cmd','extproc 'filespec("name",PERL)' -S')
    rc=lineout('reLyX\noweb2lyx.cmd')
    'sed -f ../os2config.sed reLyX/noweb2lyx.in >> reLyX\noweb2lyx.cmd'
  end
/* The extension ".cmd" is added to distinguish it from sh-script */
/* On single-user OS/2, system wide configuration is normally not necessary */
'cmd /c configure.cmd --without-latex-config & cd ..'
