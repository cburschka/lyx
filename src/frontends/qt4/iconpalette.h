// -*- C++ -*-
/**
 * \file iconpalette.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ICONPALETTE_H
#define ICONPALETTE_H

#include <QWidget>
#include <QLayout>
#include <QRect>
#include <QMenu>
#include <QWidgetItem>

#include <string>
#include <utility>
#include <vector>
#include "Action.h"

class QPushButton;

namespace lyx {
namespace frontend {

class FlowLayout : public QLayout
{
public:
	FlowLayout(QWidget *parent);
	~FlowLayout();

	void addItem(QLayoutItem *item);
	Qt::Orientations expandingDirections() const;
	bool hasHeightForWidth() const;
	int heightForWidth(int) const;
	QSize minimumSize() const;
	void setGeometry(const QRect &rect);
	QSize sizeHint() const;
	QLayoutItem * takeAt(int index);
	QLayoutItem * itemAt(int index) const;
	int count() const;

private:
	int doLayout(const QRect &rect, bool testOnly) const;
	QList<QLayoutItem *> itemList;
};


/**
 * For holding an arbitrary set of icons.
 */
class IconPalette : public QWidget {
	Q_OBJECT
public:
	IconPalette(QWidget * parent, char const ** entries);

Q_SIGNALS:
	void button_clicked(const std::string &);

protected Q_SLOTS:
	virtual void clicked();

private:
	typedef std::pair<QPushButton *, std::string> Button;
	std::vector<Button> buttons_;
};


/**
 * For holding an arbitrary set of icons.
 * is susceptible to replace IconPalette
 */
class IconPanel : public QWidget {
	Q_OBJECT
public:
	IconPanel(QWidget * parent);
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
