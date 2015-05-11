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

#include "InsetParamsWidget.h"
#include "ui_FloatPlacementUi.h"

#include "support/docstring.h"

namespace lyx {

class FloatList;
class Inset;
class InsetFloatParams;

namespace frontend {

class FloatPlacement : public InsetParamsWidget, public Ui::FloatPlacementUi {
	Q_OBJECT
public:
	FloatPlacement(bool show_options = false, QWidget * parent = 0);

	/// \name DialogView inherited methods
	//@{
	InsetCode insetCode() const { return FLOAT_CODE; }
	FuncCode creationCode() const { return LFUN_FLOAT_INSERT; }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}
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

private:
	///
	void checkAllowed() const;
	///
	std::string const get(bool & wide, bool & sideways) const;
	///
	void initFloatTypeCO(FloatList const & floats);

	/// one of figure or table?
	bool standardfloat_;
	///
	FloatList const * float_list_;
};

} // namespace frontend
} // namespace lyx

#endif // FLOATPLACEMENT_H
