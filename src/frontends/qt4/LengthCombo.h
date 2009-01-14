// -*- C++ -*-
/**
 * \file LengthCombo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LENGTHCOMBO_H
#define LENGTHCOMBO_H

#include <QComboBox>

#include "VSpace.h"


//namespace lyx {

/**
 * A combo box for selecting Length::UNIT types.
 */
class LengthCombo : public QComboBox {
	Q_OBJECT

public:
	LengthCombo(QWidget * parent);

	/// set the current item from unit
	virtual void setCurrentItem(lyx::Length::UNIT unit);
	/// set the current item from int
	virtual void setCurrentItem(int item);
	/// get the current item
	lyx::Length::UNIT currentLengthItem() const;
	/// enable the widget
	virtual void setEnabled(bool b);
	/// use the %-items?
	virtual void noPercents();
	/// remove a unit from the combo
	virtual void removeItem(lyx::Length::UNIT unit);
	/// remove an item to the combo
	virtual void removeItem(int item);
	/// add a unit to the combo
	virtual void addItem(lyx::Length::UNIT unit);
	/// add an item to the combo
	virtual void addItem(QString const item);

protected Q_SLOTS:
	virtual void has_activated(int index);
Q_SIGNALS:
	/// the current selection has changed
	void selectionChanged(lyx::Length::UNIT unit);
};


//} // namespace lyx

#endif // LENGTHCOMBO_H
