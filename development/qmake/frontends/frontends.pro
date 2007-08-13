
include(../config.pri)

TARGET = lyxfrontends$${DEBUGSUFFIX}

INC += boost
INC += src
INC += src/support

HPP += Alert_pimpl.h
HPP += Application.h
HPP += Clipboard.h
HPP += Dialogs.h
HPP += FileDialog.h
HPP += FontLoader.h
HPP += FontMetrics.h
HPP += Gui.h
HPP += KeySymbol.h
HPP += LyXView.h
HPP += Menubar.h
HPP += NoGuiFontLoader.h
HPP += NoGuiFontMetrics.h
HPP += Painter.h
HPP += Selection.h
HPP += Toolbars.h
HPP += WorkArea.h
HPP += alert.h
HPP += key_state.h
HPP += mouse_state.h

CPP += Application.cpp
CPP += Dialogs.cpp
CPP += LyXView.cpp
CPP += Painter.cpp
CPP += Toolbars.cpp
CPP += WorkArea.cpp
CPP += alert.cpp

for(FILE,CPP) { SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/frontends/$${FILE} }
for(FILE,HPP) { HEADERS += $${BUILD_BASE_SOURCE_DIR}/src/frontends/$${FILE} }
for(PATH,INC) { INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/$${PATH} }
