/**
 * \file iconpalette.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef ICONPALETTE_H
#define ICONPALETTE_H

#include <config.h>

#include "LString.h"
 
#include <qwidget.h>

#include <vector>
#include <algorithm>

class QPixmap;
class QPushButton;
class QGridLayout;

/**
 * For holding an arbitrary set of icons.
 */
class IconPalette : public QWidget {
	Q_OBJECT
public:
	IconPalette(QWidget * parent, char const * name = 0);

	/// add a button
	void add(QPixmap const & pixmap, string name, string tooltip);

signals:
	void button_clicked(string);

protected:
	virtual void resizeEvent(QResizeEvent * e);
 
protected slots:
	virtual void clicked();

private:
	int maxcol_;
 
	QGridLayout * layout_;
 
	typedef std::pair<QPushButton *, string> Button;
 
	std::vector<Button> buttons_;
};

#endif // ICONPALETTE_H
