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

#include "support/docstring.h"

namespace lyx {

class Inset;
class InsetFloatParams;

class FloatPlacement : public QWidget, public Ui::FloatPlacementUi {
	Q_OBJECT
public:
	FloatPlacement(bool show_options = false, QWidget * parent = 0);

	///
	void paramsToDialog(Inset const *);
	///
	docstring dialogToParams() const;
	///
	void useWide();
	///
	void useSideways();
	///
	void set(std::string const & placement);
	///
	std::string const get() const;

private Q_SLOTS:
	void on_defaultsCB_stateChanged(int state);
	void changedSlot();

Q_SIGNALS:
	void changed();

private:
	///
	void checkAllowed();
	///
	std::string const get(bool & wide, bool & sideways) const;

	/// one of figure or table?
	bool standardfloat_;
	///
	std::string float_type_;
};

} // namespace lyx

#endif // FLOATPLACEMENT_H
