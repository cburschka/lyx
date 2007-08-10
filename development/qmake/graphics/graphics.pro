
include(../config.pri)

TARGET = lyxgraphics$${DEBUGSUFFIX}

QT =

INC += boost
INC += src

HPP += GraphicsCache.h
HPP += GraphicsCacheItem.h
HPP += GraphicsConverter.h
HPP += GraphicsImage.h
HPP += GraphicsLoader.h
HPP += GraphicsParams.h
HPP += GraphicsTypes.h
HPP += LoaderQueue.h
HPP += PreviewImage.h
HPP += PreviewLoader.h
HPP += Previews.h

CPP += GraphicsCache.cpp
CPP += GraphicsCacheItem.cpp
CPP += GraphicsConverter.cpp
CPP += GraphicsImage.cpp
CPP += GraphicsLoader.cpp
CPP += GraphicsParams.cpp
CPP += GraphicsTypes.cpp
CPP += LoaderQueue.cpp
CPP += PreviewImage.cpp
CPP += PreviewLoader.cpp
CPP += Previews.cpp

for(FILE,CPP) { SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/graphics/$${FILE} }
for(FILE,HPP) { HEADERS += $${BUILD_BASE_SOURCE_DIR}/src/graphics/$${FILE} }
for(PATH,INC) { INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/$${PATH} }
