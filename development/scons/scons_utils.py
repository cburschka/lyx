# vi:filetype=python:expandtab:tabstop=4:shiftwidth=4
#
# file scons_utils.py
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Bo Peng
# Full author contact details are available in file CREDITS.
#
# This file defines all the utility functions for the
# scons-based build system of lyx
#

import os, sys, re, shutil, glob
from SCons.Util import *


def getVerFromConfigure(path):
    ''' get lyx version from the AC_INIT line of configure.ac,
        packed major and minor version numbers from the lyx version,
        and LYX_DATE from an AC_SUBST line.
    '''
    try:
        config = open(os.path.join(path, 'configure.ac'))
    except:
        print "Can not open configure.ac. "
        return 'x.x.x'
    # find a line like follows
    # AC_INIT(LyX,1.4.4svn,[lyx-devel@lists.lyx.org],[lyx])
    ver_pat = re.compile('AC_INIT\([^,]+,([^,]+),')
    date_pat = re.compile('AC_SUBST\(LYX_DATE, \["(.*)"\]\)')
    majmin_pat = re.compile('(\d+)\.(\d+)\..*')
    version = 'x.x.x'
    majmin = 'xx'
    date = 'Not released'
    for line in config.readlines():
        if ver_pat.match(line):
            (version,) = ver_pat.match(line).groups()
            majmin_match = majmin_pat.match(version)
            majmin = majmin_match.group(1) + majmin_match.group(2)
        if date_pat.match(line):
            (date,) = date_pat.match(line).groups()
        if version != 'x.x.x' and date != 'Not released':
            break
    return version.strip(), majmin.strip(), date.strip()


def relativePath(path, base):
    '''return relative path from base, which is usually top source dir'''
    # full pathname of path
    path1 = os.path.normpath(os.path.realpath(path)).split(os.sep)
    path2 = os.path.normpath(os.path.realpath(base)).split(os.sep)
    if path1[:len(path2)] != path2:
        print "Path %s is not under top source directory" % path
    if len(path2) == len(path1):
        return ''
    path3 = os.path.join(*path1[len(path2):]);
    # replace all \ by / such that we get the same comments on Windows and *nix
    path3 = path3.replace('\\', '/')
    return path3


def isSubDir(path, base):
    '''Whether or not path is a subdirectory of base'''
    path1 = os.path.normpath(os.path.realpath(path)).split(os.sep)
    path2 = os.path.normpath(os.path.realpath(base)).split(os.sep)
    return len(path2) <= len(path1) and path1[:len(path2)] == path2


def writeToFile(filename, lines, append = False):
    " utility function: write or append lines to filename "
    # create directory if needed
    dir = os.path.split(filename)[0]
    if dir != '' and not os.path.isdir(dir):
        os.makedirs(dir)
    if append:
        file = open(filename, 'a')
    else:
        file = open(filename, 'w')
    file.write(lines)
    file.close()


def env_subst(target, source, env):
    ''' subst variables in source by those in env, and output to target
        source and target are scons File() objects

        %key% (not key itself) is an indication of substitution
    '''
    assert len(target) == 1
    assert len(source) == 1
    target_file = file(str(target[0]), "w")
    source_file = file(str(source[0]), "r")

    contents = source_file.read()
    for k, v in env.items():
        try:
            val = env.subst('$'+k)
            # temporary fix for the \Resource backslash problem
            val = val.replace('\\', '/')
            # multi-line replacement
            val = val.replace('\n',r'\\n\\\n')
            contents = re.sub('@'+k+'@', val, contents)
        except:
            pass
    target_file.write(contents + "\n")
    target_file.close()
    #st = os.stat(str(source[0]))
    #os.chmod(str(target[0]), stat.S_IMODE(st[stat.ST_MODE]) | stat.S_IWRITE)


def env_nsis(source, target, env, for_signature):
    ''' Get nsis command line '''
    def quoteIfSpaced(str):
        if ' ' in str:
            return '"' + str + '"'
        else:
            return str
    ret = env['NSIS'] + " /V1 "
    if env.has_key('NSISFLAGS'):
        for flag in env['NSISFLAGS']:
            ret += flag
            ret += ' '
    if env.has_key('NSISDEFINES'):
        for d in env['NSISDEFINES']:
            ret += '/D'+d
            if env['NSISDEFINES'][d]:
                ret += '=' + quoteIfSpaced(env['NSISDEFINES'][d])
            ret += ' '
    # bundled?
    if '-bundle.exe' in str(target[0]):
        ret += '/DSETUPTYPE=BUNDLE '
    for s in source:
        ret += quoteIfSpaced(str(s))
    return ret


def env_toc(target, source, env):
    '''Generate target from source files'''
    # this is very tricky because we need to use installed lyx2lyx with 
    # correct lyx2lyx_version.py
    sys.path.append(env['LYX2LYX_DEST'])
    sys.path.append(env.Dir('$TOP_SRCDIR/lib/doc').abspath)
    import doc_toc
    # build toc
    doc_toc.build_toc(str(target[0]), [file.abspath for file in source])
    
    
def env_cat(target, source, env):
    '''Cat source > target. Avoid pipe to increase portability'''
    output = open(env.File(target[0]).abspath, 'w')
    for src in source:
        input = open(env.File(src).abspath)
        output.write(input.read())
        input.close()
    output.close()


def env_potfiles(target, source, env):
    '''Build po/POTFILES.in'''
    # command 
    #   grep -l '_(\".*\")' `find src \( -name '*.h' -o -name '*.cpp' -o -name '*.cpp.in' \) -print` | grep -v -e "src/support/Package.cpp$$" | sort | uniq
    # is used under *nix but windows users have to do these all in python
    target_file = open(str(target[0]), "w")
    potfiles = []
    trans = re.compile('_\(".*"\)', re.M)
    for file in source:
        rel_file = relativePath(str(file), env.subst('$TOP_SRCDIR'))
        if rel_file not in potfiles and trans.search(open(str(file)).read()):
            potfiles.append(rel_file)
    potfiles.sort()
    print >> target_file, '\n'.join(potfiles)
    target_file.close()

    
def createResFromIcon(env, icon_file, rc_file):
    ''' create a rc file with icon, and return res file (windows only) '''
    if os.name == 'nt':
        rc_name = env.File(rc_file).abspath
        dir = os.path.split(rc_name)[0]
        if not os.path.isdir(dir):
            os.makedirs(dir)
        rc = open(rc_name, 'w')
        print >> rc, 'IDI_ICON1  ICON DISCARDABLE "%s"' % \
            os.path.join(env.Dir('$TOP_SRCDIR').abspath, 'development', 'win32',
                'packaging', 'icons', icon_file).replace('\\', '\\\\')
        rc.close()
        return env.RES(rc_name)
    else:
        return []


def env_qtResource(target, source, env):
    '''Create resource.qrc'''
    qrc = open(str(target[0]), 'w')
    print >> qrc, "<!DOCTYPE RCC><RCC version='1.0'><qresource>"
    for file in source:
        rel_file = relativePath(str(file), env.subst('$TOP_SRCDIR/lib'))
        abs_file = str(file.abspath)
        print >> qrc, '<file alias="%s">%s</file>' % (rel_file, abs_file)
    print >> qrc, '</qresource></RCC>'
    qrc.close()

#
# autoconf tests
#

def checkPkgConfig(conf, version):
    ''' Return false if pkg_config does not exist, or is too old '''
    conf.Message('Checking for pkg-config...')
    ret = conf.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
    conf.Result(ret)
    return ret


def checkPackage(conf, pkg):
    ''' check if pkg is under the control of conf '''
    conf.Message('Checking for package %s...' % pkg)
    ret = conf.TryAction("pkg-config --print-errors --exists %s" % pkg)[0]
    conf.Result(ret)
    return ret


def checkMkdirOneArg(conf):
    check_mkdir_one_arg_source = """
#include <sys/stat.h>
int main()
{
    mkdir("somedir");
}
"""
    conf.Message('Checking for the number of args for mkdir... ')
    ret = conf.TryLink(check_mkdir_one_arg_source, '.c') or \
        conf.TryLink('#include <unistd.h>' + check_mkdir_one_arg_source, '.c') or \
        conf.TryLink('#include <direct.h>' + check_mkdir_one_arg_source, '.c')
    if ret:
        conf.Result('one')
    else:
        conf.Result('two')
    return ret


def checkCXXGlobalCstd(conf):
    ''' Checking the use of std::tolower or tolower '''
    check_global_cstd_source = '''
#include <cctype>
using std::tolower;
int main()
{
    return 0;
}
'''
    conf.Message('Checking for the use of global cstd... ')
    # if can not compile, define CXX_GLOBAL_CSTD
    ret = conf.TryLink(check_global_cstd_source, '.cpp')
    conf.Result(ret)
    return ret


def checkSelectArgType(conf):
    ''' Adapted from autoconf '''
    conf.Message('Checking for arg types for select... ')
    for arg234 in ['fd_set *', 'int *', 'void *']:
        for arg1 in ['int', 'size_t', 'unsigned long', 'unsigned']:
            for arg5 in ['struct timeval *', 'const struct timeval *']:
                check_select_source = '''
#if HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
extern int select (%s, %s, %s, %s, %s);
int main()
{
    return(0);
}
''' % (arg1, arg234, arg234, arg234, arg5)
                ret = conf.TryLink(check_select_source, '.c')
                if ret:
                    conf.Result(ret)
                    return (arg1, arg234, arg5)
    conf.Result('no (use default)')
    return ('int', 'int *', 'struct timeval *')


def checkBoostLibraries(conf, libs, lib_paths, inc_paths, versions, isDebug):
    ''' look for boost libraries
      libs: library names
      lib_paths: try these paths for boost libraries
      inc_paths: try these paths for boost headers
      versions:   supported boost versions
      isDebug:   if true, use debug libraries
    '''
    conf.Message('Checking for boost library %s... ' % ', '.join(libs))
    libprefix = conf.env['LIBPREFIX']
    libsuffix = '(%s|%s)' % (conf.env['LIBSUFFIX'], conf.env['SHLIBSUFFIX'])
    found_lib = False
    found_inc = False
    lib_names = []
    lib_path = None
    inc_path = None
    for path in lib_paths:
        conf.Log("Looking into %s\n" % path)
        for lib in libs:
            # get all the libs, then filter for the right library
            files = glob.glob(os.path.join(path, '%sboost_%s-*.*' % (libprefix, lib)))
            # check things like libboost_iostreams-gcc-mt-d-1_33_1.a
            if len(files) > 0:
                conf.Log("Find boost libraries: %s\n" % files)
                # runtime code includes s,g,y,d,p,n, where we should look for
                # d,g,y for debug, s,p,n for release
                lib_files = []
                if isDebug:
                    for ver in versions:
                        lib_files += filter(lambda x: re.search('%sboost_%s-\w+-mt-[^spn]+-%s%s' % (libprefix, lib, ver, libsuffix), x), files)
                else:
                    for ver in versions:
                        lib_files += filter(lambda x: re.search('%sboost_%s-\w+-mt-([^dgy]+-)*%s%s' % (libprefix, lib, ver, libsuffix), x), files)
                if len(lib_files) == 0:
                    # use alternative libraries
                    for ver in versions:
                        lib_files += filter(lambda x: re.search('%sboost_%s-[\w-]+%s%s' % (libprefix, lib, ver, libsuffix), x), files)
                if len(lib_files) > 0:
                    # get xxx-gcc-1_33_1 from /usr/local/lib/libboost_xxx-gcc-1_33_1.a
                    name = lib_files[0].split(os.sep)[-1][len(libprefix):]
                    lib_names.append(name.split('.')[0])
                    conf.Log("Qualified libraries: %s\n" % lib_names)
                else:
                    conf.Log("No qualified library is found.\n")
                    break
        if len(lib_names) == len(libs):
            found_lib = True
            lib_path = path
            break
    if not found_lib:
        if len(lib_names) == 0:
            conf.Log("No boost library is found\n")
        else:
            conf.Log("Found boost libraries: %s\n" % lib_names)
        conf.Result('no')
        return (None, None, None)
    # check version number in boost/version.hpp
    def isValidBoostDir(dir):
        version_file = os.path.join(dir, 'boost', 'version.hpp')
        if not os.path.isfile(version_file):
            return False
        version_file_content = open(version_file).read()
        version_strings = ['#define BOOST_LIB_VERSION "%s"' % ver for ver in versions]
        return True in [x in version_file_content for x in version_strings]
    # check for boost header file
    for path in inc_paths:
        conf.Log("Checking for inc path: %s\n" % path)
        if isValidBoostDir(path):
            inc_path = path
            found_inc = True
        else:   # check path/boost_1_xx_x/boost
            dirs = glob.glob(os.path.join(path, 'boost-*'))
            if len(dirs) > 0 and isValidBoostDir(dirs[0]):
                conf.Log("Checing for sub directory: %s\n" % dirs[0])
                inc_path = dirs[0]
                found_inc = True
    # return result
    if found_inc:
        conf.Result('yes')
        conf.Log('Using boost libraries %s\n' % (', '.join(lib_names)))
        return (lib_names, lib_path, inc_path)
    else:
        conf.Result('no')
        return (None, None, None)


def checkCommand(conf, cmd):
    ''' check the existence of a command
        return full path to the command, or none
    '''
    conf.Message('Checking for command %s...' % cmd)
    res = WhereIs(cmd)
    conf.Result(res is not None)
    return res


def checkNSIS(conf):
    ''' check the existence of nsis compiler, return the fullpath '''
    conf.Message('Checking for nsis compiler...')
    res = None
    if can_read_reg:
        # If we can read the registry, get the NSIS command from it
        try:
            k = RegOpenKeyEx(hkey_mod.HKEY_LOCAL_MACHINE,
                                  'SOFTWARE\\NSIS')
            val, tok = RegQueryValueEx(k,None)
            ret = val + os.path.sep + 'makensis.exe'
            if os.path.isfile(ret):
                res = '"' + ret + '"'
            else:
                res = None
        except:
            pass # Couldn't find the key, just act like we can't read the registry
    # Hope it's on the path
    if res is None:
        res = WhereIs('makensis.exe')
    conf.Result(res is not None)
    return res


def checkLC_MESSAGES(conf):
    ''' check the definition of LC_MESSAGES '''
    check_LC_MESSAGES = '''
#include <locale.h>
int main()
{
    return LC_MESSAGES;
}
'''
    conf.Message('Checking for LC_MESSAGES in locale.h... ')
    ret = conf.TryLink(check_LC_MESSAGES, '.c')
    conf.Result(ret)
    return ret


def checkIconvConst(conf):
    ''' check the declaration of iconv '''
    check_iconv_const = '''
#include <iconv.h>
// this declaration will fail when there already exists a non const char** 
// version which returns size_t
double iconv(iconv_t cd,  char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
int main() {
    return 0; 
}
'''
    conf.Message('Checking if the declaration of iconv needs const... ')
    ret = conf.TryLink(check_iconv_const, '.cpp')
    conf.Result(ret)
    return ret


def checkSizeOfWChar(conf):
    ''' check the size of wchar '''
    check_sizeof_wchar = '''
int i[ ( sizeof(wchar_t)==%d ? 1 : -1 ) ];
int main()
{
    return 0;
}
'''
    conf.Message('Checking the size of wchar_t... ')
    if conf.TryLink(check_sizeof_wchar % 2, '.cpp'):
        ret = 2
    elif conf.TryLink(check_sizeof_wchar % 4, '.cpp'):
        ret = 4
    else:
        ret = 0
    conf.Result(str(ret))
    return ret


def checkDeclaration(conf, func, headers):
    ''' check if a function is declared in given headers '''
    check_decl = '''
#include <%%s>
int main()
{
#ifndef %s
    char *p = (char *) %s;
#endif
}
''' % (func, func)
    conf.Message('Checking for the declaration of function %s... ' % func)
    ret = True in [conf.TryLink(check_decl % header, '.c') for header in headers]
    conf.Result(ret)
    return ret

    
def createConfigFile(conf, config_file,
    config_pre = '', config_post = '',
    headers = [], functions = [], declarations = [], types = [], libs = [],
    custom_tests = [], extra_items = []):
    ''' create a configuration file, with options
        config_file: which file to create
        config_pre: first part of the config file
        config_post: last part of the config file
        headers: header files to check, in the form of a list of
            ('file', 'HAVE_FILE', 'c'/'c++')
        functions: functions to check, in the form of a list of
            ('func', 'HAVE_func', 'include lines'/None)
        declarations: function declarations to check, in the form of a list of
            ('func', 'HAVE_DECL_func', header_files)
        types: types to check, in the form of a list of
            ('type', 'HAVE_TYPE', 'includelines'/None)
        libs: libraries to check, in the form of a list of
            ('lib', 'HAVE_LIB', 'LIB_NAME'). HAVE_LIB will be set if 'lib' exists,
            or any of the libs exists if 'lib' is a list of libs.
            Optionally, user can provide another key LIB_NAME, that will
            be set to the detected lib (or None otherwise).
        custom_tests: extra tests to perform, in the form of a list of
            (test (True/False), 'key', 'desc', 'true config line', 'false config line')
            If the last two are ignored, '#define key 1' '/*#undef key */'
            will be used.
        extra_items: extra configuration lines, in the form of a list of
            ('config', 'description')
    Return:
        The result of each test, as a dictioanry of
            res['XXX'] = True/False
        XXX are keys defined in each argument.
    '''
    cont = config_pre + '\n'
    result = {}
    # add to this string, in appropriate format
    def configString(lines, desc=''):
        text = ''
        if lines.strip() != '':
            if desc != '':
                text += '/* ' + desc + ' */\n'
            text += lines + '\n\n'
        return text
    #
    # headers
    for header in headers:
        description = "Define to 1 if you have the <%s> header file." % header[0]
        if (header[2] == 'c' and conf.CheckCHeader(header[0])) or \
            (header[2] == 'cxx' and conf.CheckCXXHeader(header[0])):
            result[header[1]] = 1
            cont += configString('#define %s 1' % header[1], desc = description)
        else:
            result[header[1]] = 0
            cont += configString('/* #undef %s */' % header[1], desc = description)
    # functions
    for func in functions:
        description = "Define to 1 if you have the `%s' function." % func[0]
        if conf.CheckFunc(func[0], header=func[2]):
            result[func[1]] = 1
            cont += configString('#define %s 1' % func[1], desc = description)
        else:
            result[func[1]] = 0
            cont += configString('/* #undef %s */' % func[1], desc = description)
    for decl in declarations:
        description = "Define to 1 if you have the declaration of `%s', and to 0 if you don't." % decl[0]
        if conf.CheckDeclaration(decl[0], decl[2]):
            result[decl[1]] = 1
            cont += configString('#define %s 1' % decl[1], desc = description)
        else:
            result[decl[1]] = 0
            cont += configString('/* #undef %s */' % decl[1], desc = description)
    # types
    for t in types:
        description = "Define to 1 if you have the `%s' type." % t[0]
        if conf.CheckType(t[0], includes=t[2]):
            result[t[1]] = 1
            cont += configString('#define %s 1' % t[1], desc = description)
        else:
            result[t[1]] = 0
            cont += configString('/* #undef %s */' % t[1],  desc = description)
    # libraries
    for lib in libs:
        description = "Define to 1 if you have the `%s' library (-l%s)." % (lib[0], lib[0])
        if type(lib[0]) is type(''):
            lib_list = [lib[0]]
        else:
            lib_list = lib[0]
        # check if any of the lib exists
        result[lib[1]] = 0
        # if user want the name of the lib detected
        if len(lib) == 3:
            result[lib[2]] = None
        for ll in lib_list:
            if conf.CheckLib(ll):
                result[lib[1]] = 1
                if len(lib) == 3:
                    result[lib[2]] = ll
                cont += configString('#define %s 1' % lib[1], desc = description)
                break
        # if not found
        if not result[lib[1]]:
            cont += configString('/* #undef %s */' % lib[1], desc = description)
    # custom tests
    for test in custom_tests:
        if test[0]:
            result[test[1]] = 1
            if len(test) == 3:
                cont += configString('#define %s 1' % test[1], desc = test[2])
            else:
                cont += configString(test[3], desc = test[2])
        else:
            result[test[1]] = 0
            if len(test) == 3:
                cont += configString('/* #undef %s */' % test[1], desc = test[2])
            else:
                cont += configString(test[4], desc = test[2])
    # extra items (no key is returned)
    for item in extra_items:
        cont += configString(item[0], desc = item[1])
    # add the last part
    cont += '\n' + config_post + '\n'
    # write to file
    writeToFile(config_file, cont)
    return result


def installCygwinLDScript(path):
    ''' Install i386pe.x-no-rdata '''
    ld_script = os.path.join(path, 'i386pe.x-no-rdata')
    script = open(ld_script, 'w')
    script.write('''/* specific linker script avoiding .rdata sections, for normal executables
for a reference see
http://www.cygwin.com/ml/cygwin/2004-09/msg01101.html
http://www.cygwin.com/ml/cygwin-apps/2004-09/msg00309.html
*/
OUTPUT_FORMAT(pei-i386)
SEARCH_DIR("/usr/i686-pc-cygwin/lib"); SEARCH_DIR("/usr/lib"); SEARCH_DIR("/usr/lib/w32api");
ENTRY(_mainCRTStartup)
SECTIONS
{
  .text  __image_base__ + __section_alignment__  :
  {
    *(.init)
    *(.text)
    *(SORT(.text$*))
    *(.glue_7t)
    *(.glue_7)
    ___CTOR_LIST__ = .; __CTOR_LIST__ = . ;
			LONG (-1);*(.ctors); *(.ctor); *(SORT(.ctors.*));  LONG (0);
    ___DTOR_LIST__ = .; __DTOR_LIST__ = . ;
			LONG (-1); *(.dtors); *(.dtor); *(SORT(.dtors.*));  LONG (0);
    *(.fini)
    /* ??? Why is .gcc_exc here?  */
    *(.gcc_exc)
    PROVIDE (etext = .);
    *(.gcc_except_table)
  }
  /* The Cygwin32 library uses a section to avoid copying certain data
    on fork.  This used to be named ".data".  The linker used
    to include this between __data_start__ and __data_end__, but that
    breaks building the cygwin32 dll.  Instead, we name the section
    ".data_cygwin_nocopy" and explictly include it after __data_end__. */
  .data BLOCK(__section_alignment__) :
  {
    __data_start__ = . ;
    *(.data)
    *(.data2)
    *(SORT(.data$*))
    *(.rdata)
    *(SORT(.rdata$*))
    *(.eh_frame)
    ___RUNTIME_PSEUDO_RELOC_LIST__ = .;
    __RUNTIME_PSEUDO_RELOC_LIST__ = .;
    *(.rdata_runtime_pseudo_reloc)
    ___RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
    __RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
    __data_end__ = . ;
    *(.data_cygwin_nocopy)
  }
  .rdata BLOCK(__section_alignment__) :
  {
  }
  .pdata BLOCK(__section_alignment__) :
  {
    *(.pdata)
  }
  .bss BLOCK(__section_alignment__) :
  {
    __bss_start__ = . ;
    *(.bss)
    *(COMMON)
    __bss_end__ = . ;
  }
  .edata BLOCK(__section_alignment__) :
  {
    *(.edata)
  }
  /DISCARD/ :
  {
    *(.debug$S)
    *(.debug$T)
    *(.debug$F)
    *(.drectve)
  }
  .idata BLOCK(__section_alignment__) :
  {
    /* This cannot currently be handled with grouped sections.
	See pe.em:sort_sections.  */
    SORT(*)(.idata$2)
    SORT(*)(.idata$3)
    /* These zeroes mark the end of the import list.  */
    LONG (0); LONG (0); LONG (0); LONG (0); LONG (0);
    SORT(*)(.idata$4)
    SORT(*)(.idata$5)
    SORT(*)(.idata$6)
    SORT(*)(.idata$7)
  }
  .CRT BLOCK(__section_alignment__) :
  {
    ___crt_xc_start__ = . ;
    *(SORT(.CRT$XC*))  /* C initialization */
    ___crt_xc_end__ = . ;
    ___crt_xi_start__ = . ;
    *(SORT(.CRT$XI*))  /* C++ initialization */
    ___crt_xi_end__ = . ;
    ___crt_xl_start__ = . ;
    *(SORT(.CRT$XL*))  /* TLS callbacks */
    /* ___crt_xl_end__ is defined in the TLS Directory support code */
    ___crt_xp_start__ = . ;
    *(SORT(.CRT$XP*))  /* Pre-termination */
    ___crt_xp_end__ = . ;
    ___crt_xt_start__ = . ;
    *(SORT(.CRT$XT*))  /* Termination */
    ___crt_xt_end__ = . ;
  }
  .tls BLOCK(__section_alignment__) :
  {
    ___tls_start__ = . ;
    *(.tls)
    *(.tls$)
    *(SORT(.tls$*))
    ___tls_end__ = . ;
  }
  .endjunk BLOCK(__section_alignment__) :
  {
    /* end is deprecated, don't use it */
    PROVIDE (end = .);
    PROVIDE ( _end = .);
    __end__ = .;
  }
  .rsrc BLOCK(__section_alignment__) :
  {
    *(.rsrc)
    *(SORT(.rsrc$*))
  }
  .reloc BLOCK(__section_alignment__) :
  {
    *(.reloc)
  }
  .stab BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.stab)
  }
  .stabstr BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.stabstr)
  }
  /* DWARF debug sections.
    Symbols in the DWARF debugging sections are relative to the beginning
    of the section.  Unlike other targets that fake this by putting the
    section VMA at 0, the PE format will not allow it.  */
  /* DWARF 1.1 and DWARF 2.  */
  .debug_aranges BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_aranges)
  }
  .debug_pubnames BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_pubnames)
  }
  /* DWARF 2.  */
  .debug_info BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_info) *(.gnu.linkonce.wi.*)
  }
  .debug_abbrev BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_abbrev)
  }
  .debug_line BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_line)
  }
  .debug_frame BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_frame)
  }
  .debug_str BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_str)
  }
  .debug_loc BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_loc)
  }
  .debug_macinfo BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_macinfo)
  }
  /* SGI/MIPS DWARF 2 extensions.  */
  .debug_weaknames BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_weaknames)
  }
  .debug_funcnames BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_funcnames)
  }
  .debug_typenames BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_typenames)
  }
  .debug_varnames BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_varnames)
  }
  /* DWARF 3.  */
  .debug_ranges BLOCK(__section_alignment__) (NOLOAD) :
  {
    *(.debug_ranges)
  }
}
''')
    script.close()
    return(ld_script)


def installCygwinPostinstallScript(path):
    ''' Install lyx.sh '''
    postinstall_script = os.path.join(path, 'lyx.sh')
    script = open(postinstall_script, 'w')
    script.write(r'''#!/bin/sh

# Add /usr/share/lyx/fonts to /etc/fonts/local.conf
# if it is not already there.
if [ -f /etc/fonts/local.conf ]; then
    grep -q /usr/share/lyx/fonts /etc/fonts/local.conf
    if [ $? -ne 0 ]; then
        sed 's/^<\/fontconfig>/<dir>\/usr\/share\/lyx\/fonts<\/dir>\n<\/fontconfig>/' /etc/fonts/local.conf > /etc/fonts/local.conf.tmp
        mv -f /etc/fonts/local.conf.tmp /etc/fonts/local.conf
        fc-cache /usr/share/lyx/fonts
    fi
fi
    ''')
    script.close()
    return(postinstall_script)


try:
    # these will be used under win32
    import win32file
    import win32event
    import win32process
    import win32security
except:
    # does not matter if it fails on other systems
    pass


class loggedSpawn:
    def __init__(self, env, logfile, longarg, info):
        # save the spawn system
        self.env = env
        self.logfile = logfile
        # clear the logfile (it may not exist)
        if logfile != '':
            # this will overwrite existing content.
            writeToFile(logfile, info, append=False)
        #
        self.longarg = longarg
        # get hold of the old spawn? (necessary?)
        self._spawn = env['SPAWN']

    # define new SPAWN
    def spawn(self, sh, escape, cmd, args, spawnenv):
        # get command line
        newargs = ' '.join(map(escape, args[1:]))
        cmdline = cmd + " " + newargs
        #
        # if log is not empty, write to it
        if self.logfile != '':
            # this tend to be slow (?) but ensure correct output
            # Note that cmdline may be long so I do not escape it
            try:
                # since this is not an essential operation, proceed if things go wrong here.
                writeToFile(self.logfile, cmd + " " + ' '.join(args[1:]) + '\n', append=True)
            except:
                print "Warning: can not write to log file ", self.logfile
        #
        # if the command is not too long, use the old
        if not self.longarg or len(cmdline) < 8000:
            exit_code = self._spawn(sh, escape, cmd, args, spawnenv)
        else:
            sAttrs = win32security.SECURITY_ATTRIBUTES()
            StartupInfo = win32process.STARTUPINFO()
            for var in spawnenv:
                spawnenv[var] = spawnenv[var].encode('ascii', 'replace')
            # check for any special operating system commands
            if cmd == 'del':
                for arg in args[1:]:
                    win32file.DeleteFile(arg)
                exit_code = 0
            else:
                # otherwise execute the command.
                hProcess, hThread, dwPid, dwTid = win32process.CreateProcess(None, cmdline, None, None, 1, 0, spawnenv, None, StartupInfo)
                win32event.WaitForSingleObject(hProcess, win32event.INFINITE)
                exit_code = win32process.GetExitCodeProcess(hProcess)
                win32file.CloseHandle(hProcess);
                win32file.CloseHandle(hThread);
        return exit_code


def setLoggedSpawn(env, logfile = '', longarg=False, info=''):
    ''' This function modify env and allow logging of
        commands to a logfile. If the argument is too long
        a win32 spawn will be used instead of the system one
    '''
    #
    # create a new spwn object
    ls = loggedSpawn(env, logfile, longarg, info)
    # replace the old SPAWN by the new function
    env['SPAWN'] = ls.spawn

