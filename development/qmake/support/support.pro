
include(../config.pri)

TARGET = lyxsupport$${DEBUGSUFFIX}

DEFINES += QT_NO_KEYWORDS

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

for(FILE,CPP) { SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/support/$${FILE} }
for(FILE,HPP) { HEADERS += $${BUILD_BASE_SOURCE_DIR}/src/support/$${FILE} }
for(PATH,INC) { INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/$${PATH} }
