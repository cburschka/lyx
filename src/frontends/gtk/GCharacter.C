/**
 * \file GCharacter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GCharacter.h"
#include "ghelpers.h"
#include "LColor.h"

#include "controllers/frnt_lang.h"
#include "controllers/helper_funcs.h"

#include "support/lstrings.h"

#include <libglademm.h>

using std::vector;
using std::string;

namespace lyx {
namespace frontend {


GCharacter::GCharacter(Dialog & parent)
	: GViewCB<ControlCharacter, GViewGladeB>(parent, _("Text Style"), false)
{}


class stringcolumns : public Gtk::TreeModel::ColumnRecord {
public:
	stringcolumns()
	{
		add(name);
	}

	Gtk::TreeModelColumn<Glib::ustring> name;
};


void GCharacter::PopulateComboBox(Gtk::ComboBox * combo,
                                  vector<string> const & strings)
{
	stringcolumns * cols = new stringcolumns;
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(*cols);
	vector<string>::const_iterator it = strings.begin();
	vector<string>::const_iterator end = strings.end();
	for(; it != end; ++it){
		Gtk::TreeModel::iterator iter = model->append();
		Gtk::TreeModel::Row row = *iter;
		row[cols->name] = *it;
	}
	combo->set_model(model);
	Gtk::CellRendererText * cell = Gtk::manage(new Gtk::CellRendererText);
	combo->pack_start(*cell, true);
	combo->add_attribute(*cell,"text",0);
}


void GCharacter::doBuild()
{
	string const gladeName = findGladeFile("character");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Button * button;

	// Manage the ok, apply and cancel/close buttons
	xml_->get_widget("Ok", button);
	setOK(button);
	xml_->get_widget("Apply", button);
	setApply(button);
	xml_->get_widget("Cancel", button);
	setCancel(button);

	xml_->get_widget("ToggleAll", toggleallcheck_);

	//Get combobox addresses
	xml_->get_widget("Family", familycombo_);
	xml_->get_widget("Series", seriescombo_);
	xml_->get_widget("Shape", shapecombo_);
	xml_->get_widget("Color", colorcombo_);
	xml_->get_widget("Language", languagecombo_);
	xml_->get_widget("Size", sizecombo_);
	xml_->get_widget("Misc", misccombo_);

	//Don't let the user change anything for read only documents
	bcview().addReadOnly(familycombo_);
	bcview().addReadOnly(seriescombo_);
	bcview().addReadOnly(shapecombo_);
	bcview().addReadOnly(colorcombo_);
	bcview().addReadOnly(languagecombo_);
	bcview().addReadOnly(sizecombo_);
	bcview().addReadOnly(misccombo_);
	bcview().addReadOnly(toggleallcheck_);

	//Caption/identifier pairs for the parameters
	vector<FamilyPair>   const family = getFamilyData();
	vector<SeriesPair>   const series = getSeriesData();
	vector<ShapePair>    const shape  = getShapeData();
	vector<SizePair>     const size   = getSizeData();
	vector<BarPair>      const bar    = getBarData();
	vector<ColorPair>    const color  = getColorData();
	vector<LanguagePair> const language  = getLanguageData(true);

	// Store the identifiers for later
	family_ = getSecond(family);
	series_ = getSecond(series);
	shape_  = getSecond(shape);
	size_   = getSecond(size);
	bar_    = getSecond(bar);
	color_  = getSecond(color);
	lang_   = getSecond(language);

	// Load the captions into the comboboxes
	PopulateComboBox(familycombo_, getFirst(family));
	PopulateComboBox(seriescombo_, getFirst(series));
	PopulateComboBox(shapecombo_, getFirst(shape));
	PopulateComboBox(sizecombo_, getFirst(size));
	PopulateComboBox(misccombo_, getFirst(bar));
	PopulateComboBox(colorcombo_, getFirst(color));
	PopulateComboBox(languagecombo_, getFirst(language));

	/* We use a table so that people with decent size screens don't
	* have to scroll.  However, this risks the popup being too wide
	* for people with small screens, and it doesn't scroll horizontally.
	* Hopefully this is not too wide (and hopefully gtk gets fixed).*/
	languagecombo_->set_wrap_width(3);

	//Load in the current settings
	update();

	familycombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GCharacter::onChange));
	seriescombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GCharacter::onChange));
	shapecombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GCharacter::onChange));
	sizecombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GCharacter::onChange));
	misccombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GCharacter::onChange));
	colorcombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GCharacter::onChange));
	languagecombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GCharacter::onChange));
}


void GCharacter::apply()
{
	int pos = familycombo_->get_active_row_number();
	controller().setFamily(family_[pos]);

	pos = seriescombo_->get_active_row_number();
	controller().setSeries(series_[pos]);

	pos = shapecombo_->get_active_row_number();
	controller().setShape(shape_[pos]);

	pos = sizecombo_->get_active_row_number();
	controller().setSize(size_[pos]);

	pos = misccombo_->get_active_row_number();
	controller().setBar(bar_[pos]);

	pos = colorcombo_->get_active_row_number();
	controller().setColor(color_[pos]);

	pos = languagecombo_->get_active_row_number();
	controller().setLanguage(lang_[pos]);

	bool const toggleall = toggleallcheck_->get_active();
	controller().setToggleAll(toggleall);
}


void GCharacter::update()
{
	int pos = int(findPos(family_, controller().getFamily()));
	familycombo_->set_active(pos);

	pos = int(findPos(series_, controller().getSeries()));
	seriescombo_->set_active(pos);

	pos = int(findPos(shape_, controller().getShape()));
	shapecombo_->set_active(pos);

	pos = int(findPos(size_, controller().getSize()));
	sizecombo_->set_active(pos);

	pos = int(findPos(bar_, controller().getBar()));
	misccombo_->set_active(pos);

	pos = int(findPos(color_, controller().getColor()));
	colorcombo_->set_active(pos);

	pos = int(findPos(lang_, controller().getLanguage()));
	languagecombo_->set_active(pos);

	toggleallcheck_->set_active(controller().getToggleAll());
}


void GCharacter::onChange()
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	int pos = familycombo_->get_active_row_number();
	if (family_[pos] != LyXFont::IGNORE_FAMILY)
		activate = ButtonPolicy::SMI_VALID;

	pos = seriescombo_->get_active_row_number();
	if (series_[pos] != LyXFont::IGNORE_SERIES)
		activate = ButtonPolicy::SMI_VALID;

	pos = shapecombo_->get_active_row_number();
	if (shape_[pos] != LyXFont::IGNORE_SHAPE)
		activate = ButtonPolicy::SMI_VALID;

	pos = sizecombo_->get_active_row_number();
	if (size_[pos] != LyXFont::IGNORE_SIZE)
		activate = ButtonPolicy::SMI_VALID;

	pos =  misccombo_->get_active_row_number();
	if (bar_[pos] != IGNORE)
		activate = ButtonPolicy::SMI_VALID;

	pos = colorcombo_->get_active_row_number();
	if (color_[pos] != LColor::ignore)
		activate = ButtonPolicy::SMI_VALID;

	pos = languagecombo_->get_active_row_number();
	if (lang_[pos] != "No change")
		activate = ButtonPolicy::SMI_VALID;

	bc().input(activate);
}

} // namespace frontend
} // namespace lyx
