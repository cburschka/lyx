// -*- C++ -*-
/**
 * \file floatplacement.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QT_FLOATPLACEMENT_H
#define QT_FLOATPLACEMENT_H

#include <qwidget.h>
#include <string>

class QCheckBox;
class QVBoxLayout;
class InsetFloatParams;

class FloatPlacement : public QWidget {
	Q_OBJECT

public:
	FloatPlacement(QWidget * parent, char * name);

	void useWide();
	void useSideways();

	void set(InsetFloatParams const & params);
	void set(std::string const & placement);
	void checkAllowed();

	std::string const get(bool & wide, bool & sideways) const;
	std::string const get() const;

public slots:
	void tbhpClicked();
	void heredefinitelyClicked();
	void spanClicked();
	void sidewaysClicked();
	void changedSlot();

signals:
	void changed();

private:
	QVBoxLayout * layout;

	QCheckBox * defaultsCB;
	QCheckBox * spanCB;
	QCheckBox * sidewaysCB;
	QCheckBox * ignoreCB;
	QCheckBox * pageCB;
	QCheckBox * heredefinitelyCB;
	QCheckBox * herepossiblyCB;
	QCheckBox * bottomCB;
	QCheckBox * topCB;
};

#endif
