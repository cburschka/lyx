// -*- C++ -*-
/*
 * lengthentry.h
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LENGTHENTRY_H
#define LENGTHENTRY_H

#include <qwidget.h>

#include "vspace.h"

class QVBoxLayout;
class KRestrictedLine;
class QComboBox;

#include "boost/utility.hpp"

/**
 * This widget provides a value edit and a combo box
 * for LyXLengths.
 */
class LengthEntry : public QWidget, boost::noncopyable {
   Q_OBJECT
public:
	LengthEntry(QWidget * parent = 0, const char * name = 0);
	
	~LengthEntry();

	/// get length string represented
	const string getLengthStr() const; 

	/// get the value
	double getValue() const;

	/// get the units as a string
	const string getStrUnits() const;

	/// get the units as a UNIT
	LyXLength::UNIT getUnits() const;

	/// set from a length string
	bool setFromLengthStr(const string & str);

	/// set the value
	void setValue(double value);

	/// set the value by a string 
	void setValue(const string & str);

	/// set the units from string e.g. "mm"
	bool setUnits(const string & str);
 
	/// set the units from a UNIT
	bool setUnits(LyXLength::UNIT unit);

	/// enable/disable
	virtual void setEnabled(bool enable);

private:
	KRestrictedLine *value;
	QComboBox *box;
	QVBoxLayout *topLayout;
};

#endif
