// -*- C++ -*-
/**
 * \file GCharacter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 * Based on version from xforms frontend
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GCHARACTER_H
#define GCHARACTER_H

#include "GViewBase.h"
#include "ControlCharacter.h" // for ControlCharacter enum

struct LColor_color;

namespace lyx {
namespace frontend {

/**
 * This class provides a GTK+ implementation of the Character Dialog.
 * The character dialog allows users to change the character settings
 * in their documents.
 */
class GCharacter
	: public GViewCB<ControlCharacter, GViewGladeB> {
public:
	///
	GCharacter(Dialog &);
private:
	/// Apply from dialog
	virtual void apply();

	/// Build the dialog
	virtual void doBuild();

	/// Update the dialog.
	virtual void update();

	void PopulateComboBox(Gtk::ComboBox * combo,
	                      std::vector<std::string> const & strings);

	std::vector<LyXFont::FONT_FAMILY> family_;
	std::vector<LyXFont::FONT_SERIES> series_;
	std::vector<LyXFont::FONT_SHAPE> shape_;
	std::vector<LyXFont::FONT_SIZE> size_;
	std::vector<FONT_STATE> bar_;
	std::vector<LColor_color> color_;
	std::vector<std::string> lang_;

	Gtk::ComboBox * familycombo_;
	Gtk::ComboBox * seriescombo_;
	Gtk::ComboBox * shapecombo_;
	Gtk::ComboBox * colorcombo_;
	Gtk::ComboBox * languagecombo_;
	Gtk::ComboBox * sizecombo_;
	Gtk::ComboBox * misccombo_;

	Gtk::TreeModelColumn<Glib::ustring> stringcol_;
	Gtk::TreeModel::ColumnRecord cols_;

	Gtk::CheckButton * toggleallcheck_;

	void GCharacter::onChange();
};

} // namespace frontend
} // namespace lyx

#endif
