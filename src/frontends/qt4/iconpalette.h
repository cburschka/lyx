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

#include <string>
#include <utility>
#include <vector>


class QPixmap;
class QPushButton;
class QGridLayout;
class QResizeEvent;

/**
 * For holding an arbitrary set of icons.
 */
class IconPalette : public QWidget {
	Q_OBJECT
public:
	IconPalette(QWidget * parent);

	/// add a button
	void add(QPixmap const & pixmap, std::string name, std::string tooltip);
Q_SIGNALS:
	void button_clicked(const std::string &);
protected:
	virtual void resizeEvent(QResizeEvent * e);
protected Q_SLOTS:
	virtual void clicked();
private:
	int maxcol_;

	QGridLayout * layout_;

	typedef std::pair<QPushButton *, std::string> Button;

	std::vector<Button> buttons_;
};

#endif // ICONPALETTE_H
