// -*- C++ -*-
/**
 * \file lengthcombo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef LENGTHCOMBO_H
#define LENGTHCOMBO_H

#ifdef __GNUG__
#pragma interface
#endif

#include <qcombobox.h>

#include "vspace.h"

/**
 * A combo box for selecting LyXLength::UNIT types.
 */
class LengthCombo : public QComboBox {
	Q_OBJECT
public:
	LengthCombo(QWidget * parent, char * name);

	/// set the current item
	virtual void setCurrentItem(LyXLength::UNIT unit);

	/// get the current item
	LyXLength::UNIT currentLengthItem() const;
protected slots:
	virtual void has_activated(int index);
signals:
	/// the current selection has changed
	void selectionChanged(LyXLength::UNIT unit);

};

#endif // LENGTHCOMBO_H
