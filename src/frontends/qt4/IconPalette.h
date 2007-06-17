// -*- C++ -*-
/**
 * \file IconPalette.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ICONPALETTE_H
#define ICONPALETTE_H

#include <QWidget>
#include <QMenu>
#include <QLayout>
#include "Action.h"

// FIXME: this can go when we move to Qt 4.3
#define QT_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

#if QT_VERSION >= QT_VERSION_CHECK(4, 2, 0)
#include <QWidgetAction>
#endif

namespace lyx {
namespace frontend {

/**
 * For holding an arbitrary set of icons.
 */
#if QT_VERSION >= QT_VERSION_CHECK(4, 2, 0)

class IconPalette : public QWidgetAction {
	Q_OBJECT
public:
	IconPalette(QWidget * parent);
	void addButton(QAction *);
	QWidget * createWidget(QWidget * parent);
public Q_SLOTS:
	void updateParent();
	void setIconSize(const QSize &);
Q_SIGNALS:
	void enabled(bool);
	void iconSizeChanged(const QSize &);
private:
	QList<QAction *> actions_;
	QSize size_;
};

#else

class IconPalette : public QWidget {
	Q_OBJECT
public:
	IconPalette(QWidget * parent);
	void addButton(QAction *);

public Q_SLOTS:
	void updateParent();

Q_SIGNALS:
	void triggered(QAction *);
	void visible(bool);

protected:
	void showEvent(QShowEvent * event);
	void hideEvent(QHideEvent * event);
	void paintEvent(QPaintEvent * event);

private Q_SLOTS:
	virtual void clicked(QAction *);

private:
	QGridLayout * layout_;
	QList<QAction *> actions_;
};

#endif // QT_VERSION >= QT_VERSION_CHECK(4, 2, 0)

/**
 * Popup menu for a toolbutton.
 * We need this to keep track whether
 * it is necessary to enable/disable
 * the toolbutton
 */
class ButtonMenu : public QMenu {
	Q_OBJECT
public:
	ButtonMenu(const QString & title, QWidget * parent = 0 );
	void add(QAction *);

public Q_SLOTS:
	void updateParent();

private:
	QList<QAction *> actions_;
};


} // namespace frontend
} // namespace lyx

#endif // ICONPALETTE_H
