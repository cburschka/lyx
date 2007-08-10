
include(config.pri)

TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += src/src.pro
SUBDIRS += boost/boost.pro
SUBDIRS += support/support.pro
SUBDIRS += graphics/graphics.pro
SUBDIRS += mathed/mathed.pro
SUBDIRS += frontends/frontends.pro
SUBDIRS += controllers/controllers.pro
SUBDIRS += qt4/qt4.pro
SUBDIRS += insets/insets.pro

SUBDIRS += app/app.pro
