// -*- C++ -*-
/**
 * \file GTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTABULAR_H
#define GTABULAR_H

#include "GViewBase.h"

#include "GtkLengthEntry.h"

#include "lyxlength.h"

#include <boost/scoped_ptr.hpp>
#include <vector>

namespace lyx {
namespace frontend {

class ControlTabular;

/** This class provides a GTK+ implementation of the Tabular Dialog.
 */
class GTabular
	: public GViewCB<ControlTabular, GViewGladeB> {
public:
	///
	GTabular(Dialog &);
private:

	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void doBuild();
	/// Update dialog before/whilst showing it.
	virtual void update();
	void onInput();
	void allBorders(bool borders);
	void updateSensitivity();

	bool updating_;

	// **** Current cell coords ****
	Gtk::SpinButton *cellrowspin_;
	Gtk::SpinButton *cellcolspin_;

	// **** Table tab ****
	Gtk::ComboBox *horzaligncombo_;
	Gtk::ComboBox *vertaligncombo_;
	Gtk::CheckButton *specifywidthcheck_;
	GtkLengthEntry *widthlength_;
	Gtk::Entry *latexargsentry_;

	Gtk::CheckButton *rotatetablecheck_;

	// **** This Cell tab ****
	Gtk::CheckButton *upperbordercheck_;
	Gtk::CheckButton *lowerbordercheck_;
	Gtk::CheckButton *leftbordercheck_;
	Gtk::CheckButton *rightbordercheck_;
	Gtk::CheckButton *rotatecellcheck_;
	Gtk::CheckButton *multicolcheck_;

	// **** Longtable tab ****
};

} // namespace frontend
} // namespace lyx

#endif  // GTABULAR_H
