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
	QString dialogTitle() const { return qt_("Float Settings"); }
	void paramsToDialog(Inset const *);
	docstring dialogToParams() const;
	bool checkWidgets(bool readonly) const;
	//@}
	///
	void useWide();
	///
	void useSideways();
	///
	void setPlacement(std::string const & placement);
	///
	void setAlignment(std::string const & placement);
	///
	std::string const getPlacement() const;
	///
	std::string const getAlignment() const;

private Q_SLOTS:
	void on_placementCO_currentIndexChanged(QString const &);
	void changedSlot();

private:
	///
	void checkAllowed() const;
	///
	void initFloatTypeCO(FloatList const & floats);
	///
	void initFloatPlacementCO(bool const);
	///
	bool possiblePlacement(char const & p) const;

	/// one of figure or table?
	bool standardfloat_;
	///
	std::string allowed_placement_;
	///
	bool allows_wide_;
	///
	bool allows_sideways_;
	///
	FloatList const * float_list_;
};

} // namespace frontend
} // namespace lyx

#endif // FLOATPLACEMENT_H
