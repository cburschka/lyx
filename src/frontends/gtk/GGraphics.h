// -*- C++ -*-
/**
 * \file GGraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GGRAPHICS_H
#define GGRAPHICS_H

#include "GViewBase.h"

#include "lyxlength.h"

#include <boost/scoped_ptr.hpp>
#include <vector>

namespace lyx {
namespace frontend {

class ControlGraphics;

/** This class provides a GTK+ implementation of the Graphics Dialog.
 */
class GGraphics
	: public GViewCB<ControlGraphics, GViewGladeB> {
public:
	///
	GGraphics(Dialog &);
private:

	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void doBuild();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Update the BoundingBox info.
	void updateBB(std::string const & filename, std::string const & bb_inset);

	void PopulateComboBox(Gtk::ComboBox * combo,
				  std::vector<std::string> const & strings);

	void updateComboFromLength(Gtk::ComboBox * combo,
			     LyXLength const & len);

	bool updating_;

	/// Store the LaTeX names for the rotation origins.
	std::vector<std::string> origins_;

	Gtk::TreeModelColumn<Glib::ustring> stringcol_;
	Gtk::TreeModel::ColumnRecord cols_;

	Gtk::Notebook * notebook_;

	// File Page
	Gtk::Entry * fileentry_;
	Gtk::SpinButton * displayscalespin_;
	Gtk::SpinButton * outputscalespin_;
	Gtk::Button * browsebutton_;
	Gtk::Button * editbutton_;
	Gtk::ComboBox * displaycombo_;
	Gtk::SpinButton * widthspin_;
	Gtk::SpinButton * heightspin_;
	Gtk::CheckButton * aspectcheck_;
	Gtk::ComboBox * widthunitscombo_;
	Gtk::ComboBox * heightunitscombo_;
	Gtk::RadioButton * setscalingradio_;
	Gtk::RadioButton * setsizeradio_;

	// Bounding Box Page
	Gtk::CheckButton * clipcheck_;
	Gtk::SpinButton * righttopxspin_;
	Gtk::SpinButton * righttopyspin_;
	Gtk::SpinButton * leftbottomxspin_;
	Gtk::SpinButton * leftbottomyspin_;
	Gtk::ComboBox * bbunitscombo_;
	Gtk::Button * bbfromfilebutton_;

	// Extra Page
	Gtk::SpinButton * anglespin_;
	Gtk::ComboBox * origincombo_;
	Gtk::CheckButton * subfigcheck_;
	Gtk::Entry * subfigentry_;
	Gtk::Entry * latexoptsentry_;
	Gtk::CheckButton * draftcheck_;
	Gtk::CheckButton * unzipcheck_;

	void onSizingModeChange();
	void onBrowseClicked();
	void onBBChange();
	void onBBFromFileClicked();
	void onSubFigCheckToggled();
	void onEditClicked();
	void onInput();
};

} // namespace frontend
} // namespace lyx

#endif  // GGRAPHICS_H
