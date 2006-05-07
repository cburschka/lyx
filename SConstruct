# vi:filetype=python:expandtab:tabstop=2:shiftwidth=2
#
# file SConstruct
# 
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
# 
# \author Bo Peng
# Full author contact details are available in file CREDITS.
#
#
# This is a scons based building system for lyx, you can use it as follows:
# (after of course installation of scons from www.scons.org)
#  
#    scons [frontend=qt3|gt4] [boost=included|auto] ...
#
# Where:
#   * use scons -h for details about parameters
#   * qt3 is used by default on linux, cygwin and mac
#   * qt4 is used by default on win32
#
# File layouts (Important):
#   * Unless you specify builddir=dir, building will happen
#     in $BUILDDIR = $mode/$platform-$frontend. E.g., something like
#       debug/linux-qt3/
#   * All shared libs will be put in $BUILDDIR/libs, e.g.
#       debug/linux-qt3/libs
#   * lyx executable will be in directories like debug/linux-qt3
#  
# Hints:
#   * scons --config=force
#     force re-configuration (use scons -H for details)
#   
#   * check config.log to see why config has failed
#
#   * use extra_inc_path, extra_lib_path, qt_dir, qt_inc_path
#     qt_lib_path to help locate qt and other libraries
#
#   * (Important) use scons logfile=logfile.log to enable command line
#     logging. (default is no logging)
#
# Notes:
#   * Currently, all scons does is building lyx in
#       $LYXROOT/$mode/$build_dir/
#     where $mode is debug or release, $build_dir is the build_dir name 
#     listed above
#
#   * scons install etc may be added later. Interested contributors can follow
#       http://www.scons.org/cgi-sys/cgiwrap/scons/moin.cgi/AccumulateBuilder
#     or
#       http://www.scons.org/cgi-sys/cgiwrap/scons/moin.cgi/DistTarBuilder
#     Please also see the commented out code in scons_utils.py
#       
#   * NSIS support can be found here.
#     http://www.scons.org/cgi-sys/cgiwrap/scons/moin.cgi/NsisSconsTool
#
#   * rpm support?
#     http://www.scons.org/cgi-sys/cgiwrap/scons/moin.cgi/RpmHonchoTemp
#
#   However, I decide to wait since scons seems to be standardizing these
#   features.
#

import os, sys

# scons_util defines a few utility function
import scons_utils as utils

#----------------------------------------------------------
# Required runtime environment
#----------------------------------------------------------

# FIXME: I remember lyx requires higher version of python?
EnsurePythonVersion(1, 5)
# Please use at least 0.96.91 (not 0.96.1)
EnsureSConsVersion(0, 96)

#----------------------------------------------------------
# Global definitions
#----------------------------------------------------------

# some global settings
PACKAGE_VERSION = '1.5.0svn'
DEVEL_VERSION = True
default_build_mode = 'debug'

PACKAGE = 'lyx'
PACKAGE_BUGREPORT = 'lyx-devel@lists.lyx.org'
PACKAGE_NAME = 'LyX'
PACKAGE_TARNAME = 'lyx'
PACKAGE_STRING = '%s %s' % (PACKAGE_NAME, PACKAGE_VERSION)
PROGRAM_SUFFIX = ''
config_h = os.path.join('src', 'config.h')
default_log_file = 'scons_lyx.log'

# FIXME: what is this? (They are used in src/support/package.C.in
LOCALEDIR = "../locale/"
LYX_DIR = "/usr/local/share/lyx"

# platform dependent default build_dir and other settings
#
# I know, somebody would say: 
#   This is TOTALLY wrong! Everything should be automatically
#   determined.
#
if os.name == 'nt':
  platform_name = 'win32'
  default_frontend = 'qt4'
  # boost and gettext are unlikely to be installed already
  default_boost_opt = 'included'
  default_gettext_opt = 'included'
  default_pch_opt = False
  default_with_x = False
  spell_checker = 'auto'
  # FIXME: I need to know what exactly is boost_posix
  boost_posix = False
  packaging_method = 'windows'
elif os.name == 'posix' and sys.platform != 'cygwin':
  platform_name = 'linux'
  default_frontend = 'qt3'
  # try to use system boost/gettext libraries
  default_boost_opt = 'auto'
  default_gettext_opt = 'auto'
  default_pch_opt = False
  default_with_x = True
  boost_posix = False
  packaging_method = 'posix'
elif os.name == 'posix' and sys.platform == 'cygwin':
  platform_name = 'cygwin'
  default_frontend = 'qt3'
  # force the use of cygwin/boost/gettext
  default_boost_opt = 'system'
  default_gettext_opt = 'system'
  default_pch_opt = False
  default_with_x = True
  boost_posix = True
  packaging_method = 'posix'
elif os.name == 'darwin':
  platform_name = 'mac'
  default_frontend = 'qt3'
  # to be safe
  default_boost_opt = 'included'
  default_gettext_opt = 'included'
  default_pch_opt = False
  default_with_x = False
  boost_posix = False
  packaging_method = 'msc'
else:  # unsupported system
  platform_name = 'others'
  default_frontend = 'qt3'
  # to be safe
  default_boost_opt = 'included'
  default_gettext_opt = 'included'
  default_pch_opt = False
  default_with_x = True
  boost_posix = False
  packaging_method = 'posix'

#---------------------------------------------------------
# Handling options
#----------------------------------------------------------
# Note that if you set the options via the command line, 
# they will be remembered in the file 'options.cache'
# 
# NOTE: the scons people are trying to fix scons so that
# options like --prefix will be accepted. Right now,
# we have to use the KEY=VALUE style of scons
# 
opts = Options(['options.cache', 'config.py'])
opts.AddOptions(
  # frontend, 
  EnumOption('frontend', 'Main GUI', 
    default_frontend,
    allowed_values = ('xform', 'qt3', 'qt4', 'gtk') ),
  # debug or release build
  EnumOption('mode', 'Building method', default_build_mode,
    allowed_values = ('debug', 'release') ),
  # boost libraries
  EnumOption('boost', 
    'Use included, system boost library, or try sytem first.', 
    default_boost_opt,
    allowed_values = (
      'auto',       # detect boost, if not found, use included
      'included',   # always use included boost
      'system',     # always use system boost, fail if can not find
      ) ),
  EnumOption('gettext', 
    'Use included, system gettext library, or try sytem first', 
    default_gettext_opt,
    allowed_values = (
      'auto',       # detect gettext, if not found, use included
      'included',   # always use included gettext
      'system',     # always use system gettext, fail if can not find
      ) ),
  # FIXME: I am not allowed to use '' as default, '.' is not good either.
  PathOption('qt_dir', 'Path to qt directory', '.'),
  PathOption('qt_include_path', 'Path to qt include directory', '.'),
  PathOption('qt_lib_path', 'Path to qt library directory', '.'),
  # FIXME: I do not know how pch is working. Ignore this option now.
  BoolOption('pch', '(NA) Whether or not use pch', default_pch_opt),
  # FIXME: Not implemented yet.
  BoolOption('version_suffix', '(NA) Whether or not add version suffix', False),
  # build directory, will replace build_dir if set
  PathOption('build_dir', 'Build directory', '.'),
  # extra include and libpath
  PathOption('extra_inc_path', 'Extra include path', '.'),
  PathOption('extra_lib_path', 'Extra library path', '.'),
  # enable assertion, (config.h has  ENABLE_ASSERTIOS
  BoolOption('assertions', 'Use assertions', True),
  # enable warning, (config.h has  WITH_WARNINGS)
  BoolOption('warnings', 'Use warnings', True),
  # enable glib, (config.h has  _GLIBCXX_CONCEPT_CHECKS)
  BoolOption('concept_checks', 'Enable concept checks', True),
  # FIXME: I do not know what is nls
  BoolOption('nls', '(NA) Whether or not use native language support', False),
  # FIXME: not implemented
  BoolOption('profile', '(NA) Whether or not enable profiling', False),
  # FIXME: not implemented
  PathOption('prefix', '(NA) install architecture-independent files in PREFIX', '.'),
  # FIXME: not implemented
  PathOption('exec_prefix', '(NA) install architecture-independent executable files in PREFIX', '.'),
  # FIXME: not implemented
  BoolOption('std_debug', '(NA) Whether or not turn on stdlib debug', False),
  # using x11?
  BoolOption('X11', 'Use x11 windows system', default_with_x),
  # FIXME: not implemented
  BoolOption('libintl', '(NA) Use libintl library', False),
  # FIXME: not implemented
  PathOption('intl_prefix', '(NA) Path to intl library', '.'),
  # log file
  ('logfile', 'save commands (not outputs) to logfile', default_log_file),
  # What is aiksaurus?
  BoolOption('aikasurus', 'Whether or not use aikasurus library', False),
  #
  EnumOption('spell', 'Choose spell checker to use.', 'auto',
    allowed_values = ('aspell', 'pspell', 'ispell', 'auto') )
)  

# Determine the frontend to use
frontend = ARGUMENTS.get('frontend', default_frontend)
use_X11 = ARGUMENTS.get('X11', default_with_x)

#---------------------------------------------------------
# Setting up environment
#---------------------------------------------------------

env = Environment(
  options = opts 
)

# set environment since I do not really like ENV = os.environ
env['ENV']['PATH'] = os.environ.get('PATH')
env['ENV']['HOME'] = os.environ.get('HOME')
env['ENV']['PKG_CONFIG_PATH'] = os.environ.get('PKG_CONFIG_PATH')
env['TOP_SRC_DIR'] = Dir('.').abspath

#
# frontend, mode, BUILDDIR and LOCALLIBPATH=BUILDDIR/libs
# 
env['frontend'] = frontend
env['mode'] = ARGUMENTS.get('mode', default_build_mode)
# lyx will be built to $build/build_dir so it is possible
# to build multiple build_dirs using the same source 
# $mode can be debug or release
if ARGUMENTS.has_key('build_dir'):
  build_dir = ARGUMENTS['build_dir']
  env['BUILDDIR'] = build_dir
else:
  # Determine the name of the build (platform+frontend
  build_dir = '%s-%s' % (platform_name, frontend)
  if use_X11 and platform_name == 'cygwin':
    build_dir += '-X11'
  env['BUILDDIR'] = os.path.join('$mode', build_dir)
# all built libraries will go to build_dir/libs
# (This is different from the make file approach)
env['LOCALLIBPATH'] = '#$BUILDDIR/libs'
env.Append(LIBPATH = ['$LOCALLIBPATH'])

#
# QTDIR, QT_LIB_PATH, QT_INC_PATH
#
if platform_name == 'win32':
  env.Tool('mingw')

if ARGUMENTS.has_key('qt_dir'):
  env['QTDIR'] = ARGUMENTS['qt_dir']
  # add path to the qt tools
  env.Append(LIBPATH = [os.path.join(ARGUMENTS['qt_dir'], 'lib')])
  env.Append(CPPPATH = [os.path.join(ARGUMENTS['qt_dir'], 'include')])
  # set environment so that moc etc can be found even if its path is not set properly
  env['ENV']['PATH'] = os.path.join(ARGUMENTS['qt_dir'], 'bin') + os.pathsep + env['ENV']['PATH']
else:
  env['QTDIR'] = os.environ.get('QTDIR', '/usr/lib/qt-3.3')

if ARGUMENTS.has_key('qt_lib_path'):
  env['QT_LIB_PATH'] = ARGUMENTS['qt_lib_path']
else:
  env['QT_LIB_PATH'] = '$QTDIR/lib'
env.Append(LIBPATH = ['$QT_LIB_PATH'])
# qt4 seems to be using pkg_config
env['ENV']['PKG_CONFIG_PATH'] = env.subst('$QT_LIB_PATH')

if ARGUMENTS.has_key('qt_inc_path'):
  env['QT_INC_PATH'] = ARGUMENTS['qt_inc_path']
elif os.path.isdir(os.path.join(env.subst('$QTDIR'), 'include')):
  env['QT_INC_PATH'] = '$QTDIR/include'
else: # have to guess
  env['QT_INC_PATH'] = '/usr/include/$frontend/'
env.Append(CPPPATH = env['QT_INC_PATH'])  

#
# extra_inc_path and extra_lib_path
#
if ARGUMENTS.has_key('extra_inc_path'):
  env.Append(CPPPATH = [ARGUMENTS['extra_inc_path']])
if ARGUMENTS.has_key('extra_lib_path'):
  env.Append(LIBPATH = [ARGUMENTS['extra_lib_path']])

#
# this is a bit out of place (after auto-configration)
# but it is required to do the tests.
if platform_name == 'win32':
  env.Append(CPPPATH = ['#c:/MinGW/include'])

#----------------------------------------------------------
# Autoconf business 
#----------------------------------------------------------

conf = Configure(env,
  custom_tests = {
    'CheckPkgConfig' : utils.checkPkgConfig,
    'CheckPackage' : utils.checkPackage,
    'CheckPutenv' : utils.checkPutenv,
    'CheckIstreambufIterator' : utils.checkIstreambufIterator,
    'CheckMkdirOneArg' : utils.checkMkdirOneArg,
    'CheckStdCount' : utils.checkStdCount,
    'CheckSelectArgType' : utils.checkSelectArgType,
    'CheckBoostLibraries' : utils.checkBoostLibraries,
  }
)

# pkg-config? (if not, we use hard-coded options)
if conf.CheckPkgConfig('0.15.0'):
  env['HAS_PKG_CONFIG'] = True
else:
  print 'pkg-config >= 0.1.50 is not found'
  env['HAS_PKG_CONFIG'] = False

# zlib? This is required.
if not conf.CheckLibWithHeader('z', 'zlib.h', 'C'): 
  print 'Did not find libz or zlib.h, exiting!'
  Exit(1)

# qt libraries?
#
# qt3 does not use pkg_config
if env['frontend'] == 'qt3':
  if not conf.CheckLibWithHeader('qt-mt', 'qapp.h', 'c++', 'QApplication qapp();'):
    print 'Did not find qt libraries, exiting!'
    Exit(1)
elif env['frontend'] == 'qt4':
  succ = False
  # first: try pkg_config
  if env['HAS_PKG_CONFIG']:
    succ = conf.CheckPackage('QtCore')
    env['QT4_PKG_CONFIG'] = succ
  # second: try to link to it
  if not succ:
    # FIXME: under linux, I can test the following perfectly
    # However, under windows, lib names need to passed as libXXX4.a ...
    succ = conf.CheckLibWithHeader('QtCore', 'QtGui/QApplication', 'c++', 'QApplication qapp();')
  # third: try to look up the path
  if not succ:
    succ = True
    for lib in ['QtCore', 'QtGui', 'Qt3Support']:
      # windows version has something like QtGui4 ...
      if not (os.path.isfile(os.path.join(env.subst('$QT_LIB_PATH'), 'lib%s.a' % lib)) or \
        os.path.isfile(os.path.join(env.subst('$QT_LIB_PATH'), 'lib%s4.a' % lib))):
        succ = False
        break
  # still can not find it
  if not succ:
    print 'Did not find qt libraries, exiting!'
    Exit(1)

# check socket libs
env['socket_libs'] = []
if conf.CheckLib('socket'):
  env.Append(socket_libs = ['socket'])

# FIXME: What is nsl, is it related to socket?
if conf.CheckLib('nsl'):
  env.Append(socket_libs = ['nsl'])

# check boost libraries
boost_opt = ARGUMENTS.get('boost', default_boost_opt)
# check for system boost
succ = False
if boost_opt in ['auto', 'system']:
  pathes = env['LIBPATH'] + ['/usr/lib', '/usr/local/lib']
  sig = conf.CheckBoostLibraries('boost_signals', pathes)
  reg = conf.CheckBoostLibraries('boost_regex', pathes)
  fil = conf.CheckBoostLibraries('boost_filesystem', pathes)
  ios = conf.CheckBoostLibraries('boost_iostreams', pathes)
  # if any them is not found
  if ('' in [sig[0], reg[0], fil[0], ios[0]]):
    if boost_opt == 'system':
      print "Can not find system boost libraries"
      print "Please supply a path through extra_lib_path"
      print "and try again."
      Exit(2)
  else:
    env['BOOST_LIBRARIES'] = [sig[1], reg[1], fil[1], ios[1]]
    # assume all boost libraries are in the same path...
    env.Append(LIBPATH = sig[0])
    env['INCLUDED_BOOST'] = False
    succ = True
# now, auto and succ = false, or included
if not succ:
  # we do not need to set LIBPATH now.
  env['BOOST_LIBRARIES'] = ['boost_signals', 'boost_regex', 
    'boost_filesystem', 'boost_iostreams']
  env['INCLUDED_BOOST'] = True
  
#
# Building config.h
# 

print "Generating ", config_h, "..."

# I do not handle all macros in src/config.h.in, rather I am following a list
# of *used-by-lyx* macros compiled by Abdelrazak Younes <younes.a@free.fr> 
# 
# Note: addToConfig etc are defined in scons_util
utils.startConfigH(config_h)

# HAVE_IO_H
# HAVE_LIMITS_H
# HAVE_LOCALE_H
# HAVE_LOCALE
# HAVE_PROCESS_H
# HAVE_STDLIB_H
# HAVE_SYS_STAT_H
# HAVE_SYS_TIME_H
# HAVE_SYS_TYPES_H
# HAVE_SYS_UTIME_H
# HAVE_UNISTD_H
# HAVE_UTIME_H
# HAVE_ISTREAM
# HAVE_OSTREAM
# HAVE_IOS

# Check header files
headers = [
  ('io.h', 'HAVE_IO_H', 'c'),
  ('limits.h', 'HAVE_LIMITS_H', 'c'),
  ('locale.h', 'HAVE_LOCALE_H', 'c'),
  ('locale', 'HAVE_LOCALE', 'cxx'),
  ('process.h', 'HAVE_PROCESS_H', 'c'),
  ('stdlib.h', 'HAVE_STDLIB_H', 'c'),
  ('sys/stat.h', 'HAVE_SYS_STAT_H', 'c'),
  ('sys/time.h', 'HAVE_SYS_TIME_H', 'c'),
  ('sys/types.h', 'HAVE_SYS_TYPES_H', 'c'),
  ('sys/utime.h', 'HAVE_SYS_UTIME_H', 'c'),
  ('sys/socket.h', 'HAVE_SYS_SOCKET_H', 'c'),
  ('unistd.h', 'HAVE_UNISTD_H', 'c'),
  ('utime.h', 'HAVE_UTIME_H', 'c'),
  ('istream', 'HAVE_ISTREAM', 'cxx'),
  ('ostream', 'HAVE_OSTREAM', 'cxx'),
  ('ios', 'HAVE_IOS', 'cxx')
]

for header in headers:
  if (header[2] == 'c' and conf.CheckCHeader(header[0])) or \
    (header[2] == 'cxx' and conf.CheckCXXHeader(header[0])):
    utils.addToConfig('#define %s 1' % header[1])
  else:
    utils.addToConfig('/* #undef %s */' % header[1])

# HAVE_OPEN
# HAVE_CLOSE
# HAVE_POPEN
# HAVE_PCLOSE
# HAVE__OPEN
# HAVE__CLOSE
# HAVE__POPEN
# HAVE__PCLOSE
# HAVE_GETPID
# HAVE__GETPID
# HAVE_MKDIR
# HAVE__MKDIR
# HAVE_MKTEMP
# HAVE_MKSTEMP
# HAVE_STRERROR

# Check functions
functions = [
  ('open', 'HAVE_OPEN'),
  ('close', 'HAVE_CLOSE'),
  ('popen', 'HAVE_POPEN'),
  ('pclose', 'HAVE_PCLOSE'),
  ('_open', 'HAVE__OPEN'),
  ('_close', 'HAVE__CLOSE'),
  ('_popen', 'HAVE__POPEN'),
  ('_pclose', 'HAVE__PCLOSE'),
  ('getpid', 'HAVE_GETPID'),
  ('_getpid', 'HAVE__GETPID'),
  ('mkdir', 'HAVE_MKDIR'),
  ('_mkdir', 'HAVE__MKDIR'),
  ('mktemp', 'HAVE_MKTEMP'),
  ('mkstemp', 'HAVE_MKSTEMP'),
  ('strerror', 'HAVE_STRERROR')
]

for func in functions:
  if conf.CheckFunc(func[0]):
    utils.addToConfig('#define %s 1' % func[1])
  else:
    utils.addToConfig('/* #undef %s */' % func[1])

# PACKAGE
# PACKAGE_VERSION
# DEVEL_VERSION
utils.addToConfig('#define PACKAGE "%s"' % PACKAGE)
utils.addToConfig('#define PACKAGE_VERSION "%s"' % PACKAGE_VERSION)
if DEVEL_VERSION:
  utils.addToConfig('#define DEVEL_VERSION 1')

# ENABLE_ASSERTIONS
# ENABLE_NLS
# WITH_WARNINGS
# _GLIBCXX_CONCEPT_CHECKS

# items are (ENV, ARGUMENTS)
values = [
  ('ENABLE_ASSERTIONS', 'assertions'),
  ('ENABLE_NLS', 'nls'),
  ('WITH_WARNINGS', 'warnings'),
  ('_GLIBCXX_CONCEPT_CHECKS', 'concept_checks'),
]

for val in values:
  if (env.has_key(val[0]) and env[val[0]]) or \
      ARGUMENTS.get(val[1]):
    utils.addToConfig('#define %s 1' % val[0])
  else:
    utils.addToConfig('/* #undef %s */' % val[0])

# AIKSAURUS_H_LOCATION
if ARGUMENTS.get('aiksaurus'):
  if conf.CheckLib('Aiksaurus'):
    utils.addToConfig("#define AIKSAURUS_H_LOCATION")
  else:
    print 'Library Aiksaurus not found'
    Exit(2)

# USE_ASPELL
# USE_PSPELL
# USE_ISPELL

# determine headers to use
spell_engine = ARGUMENTS.get('spell', 'auto')
spell_detected = False
if spell_engine in ['auto', 'aspell'] and \
  conf.CheckLibWithHeader('aspell', 'aspell.h', 'C'):
  utils.addToConfig('#define USE_ASPELL 1')
  spell_detected = True
elif spell_engine in ['auto', 'pspell'] and \
  conf.CheckLibWithHeader('pspell', 'pspell.h', 'C'):
  utils.addToConfig('#define USE_PSPELL 1')
  spell_detected = True
elif spell_engine in ['auto', 'ispell'] and \
  conf.CheckLibWithHeader('ispell', 'ispell.h', 'C'):
  utils.addToConfig('#define USE_ISPELL 1')
  spell_detected = False

if not spell_detected:
  # FIXME: can lyx work without an spell engine
  if spell_engine == 'auto':
    print "Warning: Can not locate any spell checker"
  else:
    print "Warning: Can not locate specified spell checker:", spell_engine

# USE_POSIX_PACKAGING
# USE_MACOSX_PACKAGING
# USE_WINDOWS_PACKAGING
if packaging_method == 'windows':
  utils.addToConfig('#define USE_WINDOWS_PACKAGING 1')
elif packaging_method == 'posix':
  utils.addToConfig('#define USE_POSIX_PACKAGING 1')
elif packaging_method == 'mac':
  utils.addToConfig('#define USE_MACOSX_PACKAGING 1')

# BOOST_POSIX
if boost_posix:
  utils.addToConfig('#define BOOST_POSIX 1')
else:
  utils.addToConfig('/* #undef BOOST_POSIX */')

# HAVE_PUTENV
if conf.CheckPutenv():
  utils.addToConfig('#define HAVE_PUTENV 1')
else:
  utils.addToConfig('/* #undef HAVE_PUTENV */')
  
# HAVE_DECL_ISTREAMBUF_ITERATOR
if conf.CheckIstreambufIterator():
  utils.addToConfig('#define HAVE_DECL_ISTREAMBUF_ITERATOR 1')
else:
  utils.addToConfig('/* #undef HAVE_DECL_ISTREAMBUF_ITERATOR */')

# MKDIR_TAKES_ONE_ARG
if conf.CheckMkdirOneArg():
  utils.addToConfig('#define MKDIR_TAKES_ONE_ARG 1')
else:
  utils.addToConfig('/* #undef MKDIR_TAKES_ONE_ARG */')

# HAVE_STD_COUNT
if conf.CheckStdCount():
  utils.addToConfig('#define HAVE_STD_COUNT 1')
else:
  utils.addToConfig('/* #undef HAVE_STD_COUNT */')

# SELECT_TYPE_ARG1
# SELECT_TYPE_ARG234
# SELECT_TYPE_ARG5
(arg1, arg234, arg5) = conf.CheckSelectArgType()
utils.addToConfig('#define SELECT_TYPE_ARG1 %s' % arg1)
utils.addToConfig('#define SELECT_TYPE_ARG234 %s' % arg234)
utils.addToConfig('#define SELECT_TYPE_ARG5 %s' % arg5)

# mkstemp
# USE_BOOST_FORMAT
# WANT_GETFILEATTRIBUTESEX_WRAPPER
utils.endConfigH(config_h)

#
# Finish auto-configuration
env = conf.Finish()

#----------------------------------------------------------
# Now set up our build process accordingly 
#----------------------------------------------------------

#
# QT_LIB etc (EXTRA_LIBS holds lib for each frontend)
#
# NOTE: Tool('qt') or Tool('qt4') will be loaded later
# in their respective directory and specialized env.
try:
  if frontend == 'qt3':
    # note: env.Tool('qt') my set QT_LIB to qt
    env['QT_LIB'] = 'qt-mt'
    env['EXTRA_LIBS'] = ['qt-mt']
    if platform_name == 'cygwin' and use_X11:
      env['EXTRA_LIBS'] += ['GL',  'Xmu', 'Xi', 'Xrender', 'Xrandr', 'Xcursor',
        'Xft', 'freetype', 'fontconfig', 'Xext', 'X11', 'SM', 'ICE', 'resolv',
        'pthread']
      env.Append(LIBPATH = ['/usr/X11R6/lib'])
  elif frontend == 'qt4':
    # local qt4 toolset from 
    # http://www.iua.upf.es/~dgarcia/Codders/sconstools.html
    env['QT_LIB'] = ['QtCore', 'QtGui', 'Qt3Support']
    env['EXTRA_LIBS'] = env['QT_LIB']
except:
  print "Can not locate qt tools"
  print "What I get is "
  print "  QTDIR: ", env['QTDIR']

#
# Build parameters CPPPATH etc
#
# boost is always in
env.Append(CPPPATH = ['#boost', '#src'])

# TODO: add (more) appropriate compiling options (-DNDEBUG etc)
# for debug/release mode 
if ARGUMENTS.get('mode', default_build_mode) == 'debug':
  env.Append(CCFLAGS = [])
else:
  env.Append(CCFLAGS = [])

#
# Customized builders
#
# install customized builders
env['BUILDERS']['substFile'] = Builder(action = utils.env_subst)
# FIXME: there must be a better way.
env['BUILDERS']['fileCopy'] = Builder(action = utils.env_filecopy)

#
# A Link script for cygwin see
# http://www.cygwin.com/ml/cygwin/2004-09/msg01101.html
# http://www.cygwin.com/ml/cygwin-apps/2004-09/msg00309.html
# for details
# 
if platform_name == 'cygwin' and env['frontend'] == 'qt3':
  ld_script_path = '/usr/lib/qt3/mkspecs/cygwin-g++'
  ld_script = utils.installCygwinLDScript(ld_script_path)
  env.Append(LINKFLAGS = ['-Wl,--enable-runtime-pseudo-reloc -Wl,--script,%s -Wl,-s' % ld_script])


#
# Report results
#
# src/support/package.C.in needs the following to replace
env['LYX_DIR'] = LYX_DIR
env['LOCALEDIR'] = LOCALEDIR
env['TOP_SRCDIR'] = str(Dir('#'))
env['PROGRAM_SUFFIX'] = PROGRAM_SUFFIX
# needed by src/version.C.in => src/version.C
env['PACKAGE_VERSION'] = PACKAGE_VERSION
# fill in the version info
env['VERSION_INFO'] = '''Configuration
  Host type:                      %s
  Special build flags:            %s
  C   Compiler:                   %s
  C   Compiler flags:             %s %s
  C++ Compiler:                   %s
  C++ Compiler LyX flags:         %s
  C++ Compiler flags:             %s %s
  Linker flags:                   %s
  Linker user flags:              %s
Build info: 
  Builing directory:              %s
  Local library directory:        %s
  Libraries pathes:               %s
  Boost libraries:                %s
Frontend: 
  Frontend:                       %s
  Packaging:                      %s
  LyX binary dir:                 FIXME
  LyX files dir:                  FIXME
''' % (platform_name, 
  env.subst('$CCFLAGS'), env.subst('$CC'),
  env.subst('$CPPFLAGS'), env.subst('$CFLAGS'),
  env.subst('$CXX'), env.subst('$CXXFLAGS'),
  env.subst('$CPPFLAGS'), env.subst('$CXXFLAGS'),
  env.subst('$LINKFLAGS'), env.subst('$LINKFLAGS'),
  env.subst('$BUILDDIR'), env.subst('$LOCALLIBPATH'),
  str(env['LIBPATH']), str(env['BOOST_LIBRARIES']),
  env['frontend'], packaging_method)

if env['frontend'] in ['qt3', 'qt4']:
  env['VERSION_INFO'] += '''  include dir:                    %s
  library dir:                    %s
  X11:                            %s
''' % (env.subst('$QT_INC_PATH'), env.subst('$QT_LIB_PATH'), use_X11)

print env['VERSION_INFO']

#
# Mingw command line may be too short for our link usage, 
# Here we use a trick from scons wiki
# http://www.scons.org/cgi-sys/cgiwrap/scons/moin.cgi/LongCmdLinesOnWin32
#
# I also would like to add logging (commands only) capacity to the
# spawn system. 
logfile = ARGUMENTS.get('logfile', default_log_file)
if logfile != '' or platform_name == 'win32':
  import time
  utils.setLoggedSpawn(env, logfile, longarg = (platform_name == 'win32'),
    info = '''# This is a log of commands used by scons to build lyx
# Time: %s 
# Command: %s
# Info: %s
''' % (time.asctime(), ' '.join(sys.argv), 
  env['VERSION_INFO'].replace('\n','\n# ')) )


#
# Cleanup stuff
#
# save options
opts.Save('options.cache', env)
# -h will print out help info
Help(opts.GenerateHelpText(env))



#----------------------------------------------------------
# Start building
#----------------------------------------------------------
Export('env')
env.BuildDir('$BUILDDIR', 'src')
print "Building all targets recursively"

client = env.SConscript('#$BUILDDIR/client/SConscript')
lyx = env.SConscript('#$BUILDDIR/SConscript')
tex2lyx = env.SConscript('#$BUILDDIR/tex2lyx/SConscript')

# avoid using full path to build them
Alias('client', client)
Alias('tex2lyx', tex2lyx)
Alias('lyx', lyx)

Default('lyx', 'tex2lyx')

print "Buinging lyx done with targets", map(str, BUILD_TARGETS)

