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

class QGridLayout;

namespace lyx {
namespace frontend {

/**
  * tear-off widget
  */
class TearOff : public QWidget {
	Q_OBJECT
public:
	TearOff(QWidget * parent);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	void mouseReleaseEvent (QMouseEvent *);
Q_SIGNALS:
	void tearOff();
protected:
	void paintEvent(QPaintEvent *);
private:
	bool highlighted_;
};


/**
 * For holding an arbitrary set of icons.
 */
class IconPalette : public QWidget {
	Q_OBJECT
public:
	IconPalette(QWidget * parent);
	void addButton(QAction *);

Q_SIGNALS:
	void triggered(QAction *);
	void visible(bool);

protected:
	void showEvent(QShowEvent * event);
	void hideEvent(QHideEvent * event);
	void paintEvent(QPaintEvent * event);

private Q_SLOTS:
	void tearOff();
	virtual void clicked(QAction *);

private:
	QGridLayout * layout_;
	QList<QAction *> actions_;
	bool tornoff_;
	TearOff * tearoffwidget_; 
};


/**
 * Popup menu for a toolbutton.
 * We need this to keep track whether
 * it is necessary to enable/disable
 * the toolbutton
 */
class ButtonMenu : public QMenu {
	Q_OBJECT
public:
	ButtonMenu(const QString & title, QWidget * parent);
	void add(QAction *);

public Q_SLOTS:
	void updateParent();

private:
	QList<QAction *> actions_;
};


} // namespace frontend
} // namespace lyx

#endif // ICONPALETTE_H
