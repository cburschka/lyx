// -*- C++ -*-
/**
 * \file ControlTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 *
 * This is pretty icky, we should really be able to use
 * ControlInset. We can't because there are no params for
 * tabular inset.
 */

#ifndef CONTROLTABULAR_H
#define CONTROLTABULAR_H

#include "ControlConnections.h"
#include "LString.h"
#include "tabular.h"

#include <boost/signals/connection.hpp>

class InsetTabular;

class ControlTabular : public ControlConnectBD {
public:

	ControlTabular(LyXView &, Dialogs &);

	///
	int getActiveCell() const;

	/// get the contained tabular
	LyXTabular const & tabular() const;

	/// return true if units should default to metric
	bool useMetricUnits() const;

	/// set a parameter
	void set(LyXTabular::Feature, string const & arg = string());

	/// slot launching dialog to an existing inset.
	void showInset(InsetTabular *);

	/// update inset
	void updateInset(InsetTabular *);

private:

	/// we can't do this ...
	virtual void apply() {};
	/// disconnect signals and hide View.
	virtual void hide();
	/// update the dialog.
	virtual void update();

	/** Instantiation of ControlConnectBD private virtual method.
	    Slot connected to update signal. */
	virtual void updateSlot(bool);

	/// show the dialog.
	void show(InsetTabular *);
	/// connect signals
	void connectInset(InsetTabular *);

	/// pointer to the inset passed through connectInset
	InsetTabular * inset_;
	/// inset::hide connection.
	boost::signals::connection ih_;

	/// is the dialog built ?
	bool dialog_built_;
};

#endif // CONTROLTABULAR_H
