// -*- C++ -*-
/**
 * \file FloatPlacement.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FLOATPLACEMENT_H
#define FLOATPLACEMENT_H

#include "ui_FloatPlacementUi.h"
#include <QWidget>

#include <string>


namespace lyx {

class InsetFloatParams;

class FloatPlacement : public QWidget, public Ui::FloatPlacementUi {
	Q_OBJECT
public:
	FloatPlacement(QWidget * parent = 0);

	void useWide();
	void useSideways();

	void set(lyx::InsetFloatParams const & params);
	void set(std::string const & placement);
	void checkAllowed();

	std::string const get(bool & wide, bool & sideways) const;
	std::string const get() const;

public Q_SLOTS:
	void tbhpClicked();
	void changedSlot();
	void on_spanCB_clicked();
	void on_heredefinitelyCB_clicked();
	void on_sidewaysCB_clicked();

Q_SIGNALS:
	void changed();

private:
	/// one of figure or table?
	bool standardfloat_;

};

} // namespace lyx

#endif // FLOATPLACEMENT_H
