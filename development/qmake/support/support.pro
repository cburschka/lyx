
include(../config.pri)

TARGET = lyxsupport$${DEBUGSUFFIX}

QT = core

INC += boost
INC += src

HPP += ExceptionMessage.h
HPP += FileFilterList.h
HPP += FileMonitor.h
HPP += FileName.h
HPP += ForkedCallQueue.h
HPP += Forkedcall.h
HPP += ForkedcallsController.h
HPP += Package.h
HPP += Path.h
HPP += RandomAccessList.h
HPP += Systemcall.h
HPP += Timeout.h
HPP += Translator.h
HPP += convert.h
HPP += copied_ptr.h
HPP += cow_ptr.h
HPP += debugstream.h
HPP += docstream.h
HPP += docstring.h
HPP += environment.h
HPP += filetools.h
HPP += fs_extras.h
HPP += gzstream.h
HPP += limited_stack.h
HPP += lstrings.h
HPP += lyxalgo.h
HPP += lyxlib.h
HPP += lyxmanip.h
HPP += lyxtime.h
HPP += os.h
HPP += os_win32.h
HPP += pch.h
HPP += qstring_helpers.h
HPP += socktools.h
HPP += std_istream.h
HPP += std_ostream.h
HPP += textutils.h
HPP += types.h
HPP += unicode.h
HPP += userinfo.h

CPP += FileFilterList.cpp
CPP += FileMonitor.cpp
CPP += FileName.cpp
CPP += ForkedCallQueue.cpp
CPP += Forkedcall.cpp
CPP += ForkedcallsController.cpp
CPP += Path.cpp
CPP += Systemcall.cpp
CPP += Timeout.cpp
CPP += abort.cpp
CPP += chdir.cpp
CPP += convert.cpp
CPP += copy.cpp
CPP += docstream.cpp
CPP += docstring.cpp
CPP += environment.cpp
CPP += filetools.cpp
CPP += fs_extras.cpp
CPP += getcwd.cpp
CPP += gzstream.cpp
CPP += kill.cpp
CPP += lstrings.cpp
CPP += lyxsum.cpp
CPP += lyxtime.cpp
CPP += mkdir.cpp
CPP += os.cpp
CPP += qstring_helpers.cpp
CPP += rename.cpp
CPP += socktools.cpp
CPP += tempname.cpp
CPP += textutils.cpp
CPP += unicode.cpp
CPP += unlink.cpp
CPP += userinfo.cpp


QMAKE_RUN_CXX1  = $(CXX) -c $(CXXFLAGS) $(INCPATH)

#packagetarget.target = Package.cpp
#packagetarget.commands = \
#	@rm -f tmp_package ;\
#	sed \'s,@LYX_DIR@,$(LYX_ABS_INSTALLED_DATADIR),;\
#s,@LOCALEDIR@,$(LYX_ABS_INSTALLED_LOCALEDIR),;\
#s,@TOP_SRCDIR@,$(LYX_ABS_TOP_SRCDIR),;\
#s,@PROGRAM_SUFFIX@,$(program_suffix),\' \
#		$${BUILD_BASE_SOURCE_DIR}/src/support/Package.cpp.in > tmp_package ;\
#	if cmp -s tmp_package Package.cpp ; then \
#		rm -f tmp_package ;\
#	else \
#		rm -f Package.cpp ;\
#		cp tmp_package Package.cpp ;\
#	fi
#packagetarget.depends = config.h
#packagetarget.variable_out = SOURCES
#packagetarget.CONFIG = no_link
##SOURCES += $${BUILD_BASE_TARGET_DIR}/src/Package.cpp
#QMAKE_EXTRA_TARGETS += packagetarget
#
##OBJECTS += $(OBJECTS_DIR)/Package.o 
#POST_TARGETDEPS += $(OBJECTS_DIR)/Package.o 

packagetarget.target = Package.cpp
packagetarget.commands = \
	@rm -f tmp_package ;\
	sed \'s,@LYX_DIR@,$(LYX_ABS_INSTALLED_DATADIR),;\
s,@LOCALEDIR@,$(LYX_ABS_INSTALLED_LOCALEDIR),;\
s,@TOP_SRCDIR@,$(LYX_ABS_TOP_SRCDIR),;\
s,@PROGRAM_SUFFIX@,$(program_suffix),\' \
		$${BUILD_BASE_SOURCE_DIR}/src/support/Package.cpp.in > tmp_package ;\
	if cmp -s tmp_package Package.cpp ; then \
		rm -f tmp_package ;\
	else \
		rm -f Package.cpp ;\
		cp tmp_package Package.cpp ;\
	fi
#packagetarget.depends = config.h
packagetarget.CONFIG = no_link
#SOURCES += $${BUILD_BASE_TARGET_DIR}/src/Package.cpp
 
packagetarget2.target = $(OBJECTS_DIR)/Package.o
packagetarget2.commands = $${QMAKE_RUN_CXX1} -c Package.cpp \
	-o $(OBJECTS_DIR)/Package.o
packagetarget2.depends = Package.cpp
 
QMAKE_EXTRA_TARGETS += packagetarget packagetarget2
 
QMAKE_CLEAN += $(OBJECTS_DIR)/Package.o Package.cpp

PRE_TARGETDEPS += $(OBJECTS_DIR)/Package.o 

LIBS += $(OBJECTS_DIR)/Package.o

for(FILE,CPP) { SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/support/$${FILE} }
for(FILE,HPP) { HEADERS += $${BUILD_BASE_SOURCE_DIR}/src/support/$${FILE} }
for(PATH,INC) { INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/$${PATH} }
