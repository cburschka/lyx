/**
 * \file qfontexample.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <qwidget.h>
#include <qfont.h>

class QFontExample : public QWidget {

public:
	QFontExample(QWidget * parent, const char * name)
		: QWidget(parent, name) {}
 
	void set(QFont const & font, QString const & text);
 
	virtual QSize sizeHint() const;
 
protected:
	virtual void paintEvent(QPaintEvent * p);
 
private:
	QFont font_;
	QString text_;
};
