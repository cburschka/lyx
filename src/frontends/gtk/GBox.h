// -*- C++ -*-
/**
 * \file GBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GBOX_H
#define GBOX_H

#include "GViewBase.h"

#include "lyxlength.h"

#include <vector>
#include <string>

namespace lyx {
namespace frontend {

class ControlBox;

class GBox
	: public GViewCB<ControlBox, GViewGladeB>
{
public:
	GBox(Dialog &);

	// This dialog is instant-apply
	virtual void apply() {}
	// update
	virtual void update();
	// build the dialog
	virtual void doBuild();

	// Put strings into combo
	void PopulateComboBox(Gtk::ComboBox * combo,
				  std::vector<std::string> const & strings);

	// add or remove special lengths in widthunits combo
	void setSpecial(bool ibox);
	// only show valid inner box options
	void updateInnerBoxCombo();

	// Signal handlers
	void onTypeComboChanged();
	void onInnerBoxComboChanged();
	void onAlignChanged();
	void onHeightChanged();
	void onWidthChanged();

	// Some event handlers are disabled when this is true
	bool applylock_;

	// The 'type' field keys and display strings
	std::vector<std::string> ids_;
	std::vector<std::string> gui_names_;

	// The special units for width and height
	std::vector<std::string> ids_spec_;
	std::vector<std::string> gui_names_spec_;

	// Widgets from glade
	Gtk::ComboBox * typecombo_;
	Gtk::ComboBox * innerboxcombo_;
	Glib::RefPtr<Gtk::ListStore> innerboxstore_;
	Gtk::ComboBox * widthunitscombo_;
	Glib::RefPtr<Gtk::ListStore> widthunitsstore_;
	Gtk::ComboBox * heightunitscombo_;
	Gtk::ComboBox * boxvertcombo_;
	Gtk::ComboBox * contentvertcombo_;
	Gtk::ComboBox * contenthorzcombo_;
	Gtk::SpinButton * widthspin_;
	Gtk::SpinButton * heightspin_;

	// Treemodel objects for use in combobox liststores
	Gtk::TreeModelColumn<Glib::ustring> stringcol_;
	Gtk::TreeModel::ColumnRecord cols_;
};

} // namespace frontend
} // namespace lyx

#endif // GBOX_H
