// -*- C++ -*-
/**
 * \file GuiFontExample.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QT_QFONTEXAMPLE_H
#define QT_QFONTEXAMPLE_H

#include <QWidget>
#include <QFont>
#include <QString>

class QPaintEvent;


//namespace lyx {

class GuiFontExample : public QWidget {

public:
	GuiFontExample(QWidget * parent)
		: QWidget(parent) {}

	void set(QFont const & font, QString const & text);

	virtual QSize sizeHint() const;

protected:
	virtual void paintEvent(QPaintEvent * p);

private:
	QFont font_;
	QString text_;
};


//} // namespace lyx

#endif
