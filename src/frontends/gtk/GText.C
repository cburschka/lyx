/**
 * \file GText.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GText.h"
#include "ControlCommand.h"
#include "ghelpers.h"
#include "IdSc.h"

#include "support/lstrings.h"

#include <gtkmm.h>
#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

GText::GText(Dialog & parent, string const & title, string const & label)
	: GViewCB<ControlCommand, GViewGladeB>(parent, title),
	  label_(label), entry_(0)
{
}


void GText::apply()
{
	string const contents = Glib::locale_from_utf8(entry_->get_text());
	controller().params().setContents(contents);
}


void GText::update()
{
	string const contents = support::trim(
		controller().params().getContents());
	entry_->set_text(Glib::locale_to_utf8(contents));
}


void GText::doBuild()
{
	string const gladeName = findGladeFile("text");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Label * label;
	Gtk::Button * restore;
	Gtk::Button * cancel;
	Gtk::Button * apply;
	Gtk::Button * ok;
	xml_->get_widget("Label", label);
	xml_->get_widget("Text", entry_);
	xml_->get_widget("Restore", restore);
	xml_->get_widget("Cancel", cancel);
	xml_->get_widget("Apply", apply);
	xml_->get_widget("OK", ok);
	label->set_text(Glib::locale_to_utf8(id_sc::id(label_)));
	setOK(ok);
	setApply(apply);
	setCancel(cancel);
	setRestore(restore);
	bcview().addReadOnly(entry_);
	entry_->signal_changed().connect(
		sigc::mem_fun(*this, &GText::onEntryChanged));
}


void GText::onEntryChanged()
{
	bc().valid(!entry_->get_text().empty());
}

} // namespace frontend
} // namespace lyx
