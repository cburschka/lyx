# TODO: set correct path
#set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

message(STATUS "installing to ${CMAKE_INSTALL_PREFIX}, defined by CMAKE_INSTALL_PREFIX")

# the macro scans the directories "_parent_src_dir/_dir/_current_dir" for *._file_type files 
# and installs the files in CMAKE_INSTALL_PREFIX/_current_dir
# dir_item is on item of the remaining arguments
macro(lyx_install _parent_src_dir _dir _file_type)
   	foreach(_current_dir ${ARGN})
   		file(GLOB files_list ${_parent_src_dir}/${_dir}/${_current_dir}/*.${_file_type})
   		list(REMOVE_ITEM files_list "${_parent_src_dir}/${_dir}/${_current_dir}/.svn")
   		install(FILES ${files_list} DESTINATION ${_dir}/${_current_dir})
   		#message(STATUS "install ${_dir}/${_current_dir}: ${files_list} ")
   		#message(STATUS "install at ${CMAKE_INSTALL_PREFIX}/${_dir}/${_current_dir}")
   	endforeach(_current_dir)  	 
endmacro(lyx_install)


lyx_install(${TOP_SRC_DIR}/lib bind         bind   . de fi pt sv)
lyx_install(${TOP_SRC_DIR}/lib commands     def    .)
lyx_install(${TOP_SRC_DIR}/lib doc          lyx    . cs da de es eu fr he hu it nl nb pl pt ro ru sk sl sv)
lyx_install(${TOP_SRC_DIR}/lib doc          *      clipart)
lyx_install(${TOP_SRC_DIR}/lib doc/es       *      clipart)
lyx_install(${TOP_SRC_DIR}/lib examples     *      . ca cs da de es eu fr he hu it nl pl pt ro ru sl)
lyx_install(${TOP_SRC_DIR}/lib fonts        *      .)
lyx_install(${TOP_SRC_DIR}/lib images       *      . math commands)
lyx_install(${TOP_SRC_DIR}/lib kbd          *      .)
lyx_install(${TOP_SRC_DIR}/lib layouts      *      .)
lyx_install(${TOP_SRC_DIR}/lib lyx2lyx      *      .)
lyx_install(${TOP_SRC_DIR}/lib scripts      *      .)
lyx_install(${TOP_SRC_DIR}/lib templates    *      .)
lyx_install(${TOP_SRC_DIR}/lib tex          *      .)
lyx_install(${TOP_SRC_DIR}/lib ui           *      .)
lyx_install(${TOP_SRC_DIR}/lib .            *      .)

# TODO also get dot-less filenames in lyx_install
foreach(_file unicodesymbols encodings languages lyx2lyx/lyx2lyx)
	install(FILES ${TOP_SRC_DIR}/lib/${_file} DESTINATION .)
endforeach(_file)

 


#  
#  cmake doku  and  scon code
#
#    remove later.
#
#  INSTALL: Specify rules to run at install time.
#  This command generates installation rules for a project. Rules specified by calls to this 
#  command within a source directory are executed in order during installation. The order across 
#  directories is not defined.
#  There are multiple signatures for this command. Some of them define installation properties 
#  for files and targets. Properties common to multiple signatures are covered here but they are 
#  valid only for signatures that specify them. DESTINATION arguments specify the directory 
#  on disk to which a file will be installed. If a full path (with a leading slash or drive letter) is 
#  given it is used directly. If a relative path is given it is interpreted relative to the value of 
#  CMAKE_INSTALL_PREFIX. PERMISSIONS arguments specify permissions for installed 
#  files. Valid permissions are OWNER_READ, OWNER_WRITE, OWNER_EXECUTE, 
#  GROUP_READ, GROUP_WRITE, GROUP_EXECUTE, WORLD_READ, 
#  WORLD_WRITE, WORLD_EXECUTE, SETUID, and SETGID. Permissions that do not 
#  make sense on certain platforms are ignored on those platforms. The CONFIGURATIONS 
#  argument specifies a list of build configurations for which the install rule applies (Debug, 
#  Release, etc.). The COMPONENT argument specifies an installation component name with 
#  which the install rule is associated, such as "runtime" or "development". During component-
#  specific installation only install rules associated with the given component name will be 
#  executed. During a full installation all components are installed. The RENAME argument 
#  specifies a name for an installed file that may be different from the original file. Renaming is 
#  allowed only when a single file is installed by the command. 
#  The TARGETS signature:
#    INSTALL(TARGETS targets... [[ARCHIVE|LIBRARY|RUNTIME] 
#                                [DESTINATION <dir>] 
#                                [PERMISSIONS permissions...] 
#                                [CONFIGURATIONS [Debug|Release|...]] 
#                                [COMPONENT <component>] 
#                               ] [...])
#  The TARGETS form specifies rules for installing targets from a project. There are three kinds 
#  of target files that may be installed: archive, library, and runtime. Executables are always 
#  treated as runtime targets. Static libraries are always treated as archive targets. Module 
#  libraries are always treated as library targets. For non-DLL platforms shared libraries are 
#  treated as library targets. For DLL platforms the DLL part of a shared library is treated as a 
#  runtime target and the corresponding import library is treated as an archive target. All 
#  Windows-based systems including Cygwin are DLL platforms. The ARCHIVE, LIBRARY, 
#  and RUNTIME arguments change the type of target to which the subsequent properties apply. 
#  If none is given the installation properties apply to all target types. If only one is given then 
#  only targets of that type will be installed (which can be used to install just a DLL or just an 
#  import library).
#  One or more groups of properties may be specified in a single call to the TARGETS form of 
#  this command. A target may be installed more than once to different locations. Consider 
#  hypothetical targets "myExe", "mySharedLib", and "myStaticLib". The code
#      INSTALL(TARGETS myExe mySharedLib myStaticLib 
#              RUNTIME DESTINATION bin 
#              LIBRARY DESTINATION lib 
#              ARCHIVE DESTINATION lib/static) 
#      INSTALL(TARGETS mySharedLib DESTINATION /some/full/path)
#  will install myExe to <prefix>/bin and myStaticLib to <prefix>/lib/static. On non-DLL 
#  platforms mySharedLib will be installed to <prefix>/lib and /some/full/path. On DLL 
#  platforms the mySharedLib DLL will be installed to <prefix>/bin and /some/full/path and its 
#  import library will be installed to <prefix>/lib/static and /some/full/path. On non-DLL 
#  platforms mySharedLib will be installed to <prefix>/lib and /some/full/path.
#  The FILES signature:
#    INSTALL(FILES files... DESTINATION <dir> 
#            [PERMISSIONS permissions...] 
#            [CONFIGURATIONS [Debug|Release|...]] 
#            [COMPONENT <component>] 
#            [RENAME <name>])
#  The FILES form specifies rules for installing files for a project. File names given as relative 
#  paths are interpreted with respect to the current source directory. Files installed by this form 
#  are by default given permissions OWNER_WRITE, OWNER_READ, GROUP_READ, and 
#  WORLD_READ if no PERMISSIONS argument is given.
#  The PROGRAMS signature:
#    INSTALL(PROGRAMS files... DESTINATION <dir> 
#            [PERMISSIONS permissions...] 
#            [CONFIGURATIONS [Debug|Release|...]] 
#            [COMPONENT <component>] 
#            [RENAME <name>])
#  The PROGRAMS form is identical to the FILES form except that the default permissions for 
#  the installed file also include OWNER_EXECUTE, GROUP_EXECUTE, and 
#  WORLD_EXECUTE. This form is intended to install programs that are not targets, such as 
#  shell scripts. Use the TARGETS form to install targets built within the project.
#  The SCRIPT and CODE signature:
#    INSTALL([[SCRIPT <file>] [CODE <code>]] [...])
#  The SCRIPT form will invoke the given CMake script files during installation. If the script 
#  file name is a relative path it will be interpreted with respect to the current source directory. 
#  The CODE form will invoke the given CMake code during installation. Code is specified as a 
#  single argument inside a double-quoted string. For example, the code
#    INSTALL(CODE "MESSAGE(\"Sample install message.\")")
#  will print a message during installation.
#  NOTE: This command supercedes the INSTALL_TARGETS command and the target 
#  properties PRE_INSTALL_SCRIPT and POST_INSTALL_SCRIPT. It also replaces the 
#  FILES forms of the INSTALL_FILES and INSTALL_PROGRAMS commands. The 
#  processing order of these install rules relative to those generated by INSTALL_TARGETS, 
#  INSTALL_FILES, and INSTALL_PROGRAMS commands is not defined.
#  
#  
#  
#  
#  
#  
#  
#  
#  if build_install:
#      #
#      # this part is a bit messy right now. Since scons will provide
#      # --DESTDIR option soon, at least the dest_dir handling can be 
#      # removed later.
#      #
#      # how to join dest_dir and prefix
#      def joinPaths(path1, path2):
#          ''' join path1 and path2, do not use os.path.join because
#              under window, c:\destdir\d:\program is invalid '''
#          if path1 == '':
#              return os.path.normpath(path2)
#          # separate drive letter
#          (drive, path) = os.path.splitdrive(os.path.normpath(path2))
#          # ignore drive letter, so c:\destdir + c:\program = c:\destdir\program
#          return os.path.join(os.path.normpath(path1), path[1:])
#      #
#      # install to dest_dir/prefix
#      dest_dir = env.get('DESTDIR', '')
#      dest_prefix_dir = joinPaths(dest_dir, env.Dir(prefix).abspath)
#      # create the directory if needed
#      if not os.path.isdir(dest_prefix_dir):
#          try:
#              os.makedirs(dest_prefix_dir)
#          except:
#              pass
#          if not os.path.isdir(dest_prefix_dir):
#              print 'Can not create directory', dest_prefix_dir
#              Exit(3)
#      #
#      if env.has_key('exec_prefix'):
#          bin_dest_dir = joinPaths(dest_dir, Dir(env['exec_prefix']).abspath)
#      else:
#          bin_dest_dir = os.path.join(dest_prefix_dir, 'bin')
#      if add_suffix:
#          share_dest_dir = os.path.join(dest_prefix_dir, share_dir + program_suffix)
#      else:
#          share_dest_dir = os.path.join(dest_prefix_dir, share_dir)
#      man_dest_dir = os.path.join(dest_prefix_dir, man_dir)
#      locale_dest_dir = os.path.join(dest_prefix_dir, locale_dir)
#      env['LYX2LYX_DEST'] = os.path.join(share_dest_dir, 'lyx2lyx')
#      #
#      import glob
#      #
#      # install executables (lyxclient may be None)
#      #
#      if add_suffix:
#          version_suffix = program_suffix
#      else:
#          version_suffix = ''
#      #
#      # install lyx, if in release mode, try to strip the binary
#      if env.has_key('STRIP') and env['STRIP'] is not None and mode != 'debug':
#          # create a builder to strip and install
#          env['BUILDERS']['StripInstallAs'] = Builder(action='$STRIP $SOURCE -o $TARGET')
#  
#      # install executables
#      for (name, obj) in (('lyx', lyx), ('tex2lyx', tex2lyx), ('client', client)):
#          if obj is None:
#              continue
#          target_name = os.path.split(str(obj[0]))[1].replace(name, '%s%s' % (name, version_suffix))
#          target = os.path.join(bin_dest_dir, target_name)
#          if env['BUILDERS'].has_key('StripInstallAs'):
#              env.StripInstallAs(target, obj)
#          else:
#              env.InstallAs(target, obj)
#          Alias('install', target)
#  
#      # share/lyx
#      dirs = []
#      for (dir,files) in [
#              ('.', lib_files),  
#              ('bind', lib_bind_files),
#              ('bind/de', lib_bind_de_files),
#              ('bind/fi', lib_bind_fi_files),
#              ('bind/pt', lib_bind_pt_files),
#              ('bind/sv', lib_bind_sv_files),
#              ('doc', lib_doc_files),
#              ('doc/clipart', lib_doc_clipart_files),
#              ('doc/cs', lib_doc_cs_files),
#              ('doc/da', lib_doc_da_files),
#              ('doc/de', lib_doc_de_files),
#              ('doc/es', lib_doc_es_files),
#              ('doc/es/clipart', lib_doc_es_clipart_files),
#              ('doc/eu', lib_doc_eu_files),
#              ('doc/fr', lib_doc_fr_files),
#              ('doc/he', lib_doc_he_files),
#              ('doc/hu', lib_doc_hu_files),
#              ('doc/it', lib_doc_it_files),
#              ('doc/nl', lib_doc_nl_files),
#              ('doc/nb', lib_doc_nb_files),
#              ('doc/pl', lib_doc_pl_files),
#              ('doc/pt', lib_doc_pt_files),
#              ('doc/ro', lib_doc_ro_files),
#              ('doc/ru', lib_doc_ru_files),
#              ('doc/sk', lib_doc_sk_files),
#              ('doc/sl', lib_doc_sl_files),
#              ('doc/sv', lib_doc_sv_files),
#              ('examples', lib_examples_files),
#              ('examples/ca', lib_examples_ca_files),
#              ('examples/cs', lib_examples_cs_files),
#              ('examples/da', lib_examples_da_files),
#              ('examples/de', lib_examples_de_files),
#              ('examples/es', lib_examples_es_files),
#              ('examples/eu', lib_examples_eu_files),
#              ('examples/fr', lib_examples_fr_files),
#              ('examples/he', lib_examples_he_files),
#              ('examples/hu', lib_examples_hu_files),
#              ('examples/it', lib_examples_it_files),
#              ('examples/nl', lib_examples_nl_files),
#              ('examples/pl', lib_examples_pl_files),
#              ('examples/pt', lib_examples_pt_files),
#              ('examples/ru', lib_examples_ru_files),
#              ('examples/sl', lib_examples_sl_files),
#              ('examples/ro', lib_examples_ro_files),
#              ('fonts', lib_fonts_files),
#              ('images', lib_images_files),
#              ('images/math', lib_images_math_files),
#              ('kbd', lib_kbd_files),
#              ('layouts', lib_layouts_files),
#              ('lyx2lyx', lib_lyx2lyx_files),
#              ('scripts', lib_scripts_files),
#              ('templates', lib_templates_files),
#              ('tex', lib_tex_files),
#              ('ui', lib_ui_files)]:
#          dirs.append(env.Install(os.path.join(share_dest_dir, dir),
#              [env.subst('$TOP_SRCDIR/lib/%s/%s' % (dir, file)) for file in files]))
#      Alias('install', dirs)
#  
#      # subst and install lyx2lyx_version.py which is not in scons_manifest.py
#      env.Depends(share_dest_dir + '/lyx2lyx/lyx2lyx_version.py', '$BUILDDIR/common/config.h')
#      env.substFile(share_dest_dir + '/lyx2lyx/lyx2lyx_version.py',
#          '$TOP_SRCDIR/lib/lyx2lyx/lyx2lyx_version.py.in')
#      Alias('install', share_dest_dir + '/lyx2lyx/lyx2lyx_version.py')
#      sys.path.append(share_dest_dir + '/lyx2lyx')
#      
#      # generate TOC files for each doc
#      languages = depend.all_documents(env.Dir('$TOP_SRCDIR/lib/doc').abspath)
#      tocs = []
#      for lang in languages.keys():
#          if os.path.isdir(os.path.join(env.Dir('$TOP_SRCDIR/lib/doc').abspath, lang)):
#              toc = env.installTOC(os.path.join(share_dest_dir, 'doc', lang, 'TOC.lyx'),
#                  languages[lang])
#              tocs.append(toc)
#              # doc_toc.build_toc needs a installed version of lyx2lyx to execute
#              env.Depends(toc, share_dest_dir + '/lyx2lyx/lyx2lyx_version.py')
#          else:
#              # this is for English
#              toc = env.installTOC(os.path.join(share_dest_dir, 'doc', 'TOC.lyx'),
#                  languages[lang])
#              tocs.append(toc)
#              env.Depends(toc, share_dest_dir + '/lyx2lyx/lyx2lyx_version.py')
#      Alias('install', tocs)
#      
#      if platform_name == 'cygwin':
#          # cygwin packaging requires a file /usr/share/doc/Cygwin/foot-vendor-suffix.README
#          Cygwin_README = os.path.join(dest_prefix_dir, 'share', 'doc', 'Cygwin', 
#              '%s-%s.README' % (package, package_cygwin_version))
#          env.InstallAs(Cygwin_README,
#              os.path.join(env.subst('$TOP_SRCDIR'), 'README.cygwin'))
#          Alias('install', Cygwin_README)
#          # also a directory /usr/share/doc/lyx for README etc
#          Cygwin_Doc = os.path.join(dest_prefix_dir, 'share', 'doc', package)
#          env.Install(Cygwin_Doc, [os.path.join(env.subst('$TOP_SRCDIR'), x) for x in \
#              ['INSTALL', 'README', 'README.Cygwin', 'RELEASE-NOTES', 'COPYING', 'ANNOUNCE']])
#          Alias('install', Cygwin_Doc)
#          # cygwin fonts also need to be installed
#          Cygwin_fonts = os.path.join(share_dest_dir, 'fonts')
#          env.Install(Cygwin_fonts, 
#              [env.subst('$TOP_SRCDIR/development/Win32/packaging/bakoma/%s' % file) \
#                    for file in win32_bakoma_fonts])
#          Alias('install', Cygwin_fonts)
#          # we also need a post installation script
#          tmp_script = utils.installCygwinPostinstallScript('/tmp')
#          postinstall_path = os.path.join(dest_dir, 'etc', 'postinstall')
#          env.Install(postinstall_path, tmp_script)
#          Alias('install', postinstall_path)
#  
#  
#      # man
#      env.InstallAs(os.path.join(man_dest_dir, 'lyx' + version_suffix + '.1'),
#          env.subst('$TOP_SRCDIR/lyx.man'))
#      env.InstallAs(os.path.join(man_dest_dir, 'tex2lyx' + version_suffix + '.1'),
#          env.subst('$TOP_SRCDIR/src/tex2lyx/tex2lyx.man'))
#      env.InstallAs(os.path.join(man_dest_dir, 'lyxclient' + version_suffix + '.1'),
#          env.subst('$TOP_SRCDIR/src/client/lyxclient.man'))
#      Alias('install', [os.path.join(man_dest_dir, x + version_suffix + '.1') for
#          x in ['lyx', 'tex2lyx', 'lyxclient']])
#      # locale files?
#      # ru.gmo ==> ru/LC_MESSAGES/lyxSUFFIX.mo
#      for gmo in gmo_files:
#          lan = os.path.split(str(gmo))[1].split('.')[0]
#          dest_file = os.path.join(locale_dest_dir, lan, 'LC_MESSAGES', 'lyx' + program_suffix + '.mo')
#          env.InstallAs(dest_file, gmo)
#          Alias('install', dest_file)
#  


