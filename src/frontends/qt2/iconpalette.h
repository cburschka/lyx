// -*- C++ -*-
/**
 * \file iconpalette.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef ICONPALETTE_H
#define ICONPALETTE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <qwidget.h>
#include "LString.h"
#include <utility>
#include <vector>

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
	void button_clicked(string const &);
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
