/**
 * \file lengthcombo.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef LENGTHCOMBO_H
#define LENGTHCOMBO_H

#include <config.h>
 
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

#endif
