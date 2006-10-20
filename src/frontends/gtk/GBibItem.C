/**
 * \file GBibItem.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GBibItem.h"
#include "ControlCommand.h"
#include "ghelpers.h"

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

GBibItem::GBibItem(Dialog & parent)
	: GViewCB<ControlCommand, GViewGladeB>(parent, _("Bibliography Entry Settings"), false)
{}


void GBibItem::doBuild()
{
	string const gladeName = findGladeFile("bibitem");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Cancel", button);
	setCancel(button);
	xml_->get_widget("OK", button);
	setOK(button);

	xml_->get_widget("Key", keyentry_);
	xml_->get_widget("Label", labelentry_);

	keyentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GBibItem::changed));
	labelentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GBibItem::changed));

	bcview().addReadOnly(keyentry_);
	bcview().addReadOnly(labelentry_);
}


void GBibItem::update()
{
	bc().refreshReadOnly();

	keyentry_->set_text (lyx::to_utf8(controller().params()["key"]));
	labelentry_->set_text (lyx::to_utf8(controller().params()["label"]));
}


void GBibItem::apply()
{
	controller().params()["key"] = lyx::from_utf8(keyentry_->get_text());
	controller().params()["label"] = lyx::from_utf8(labelentry_->get_text());
}

void GBibItem::changed()
{
	if (keyentry_->get_text().size() > 0)
		bc().valid(TRUE);
	else
		bc().valid(FALSE);
}

} // namespace frontend
} // namespace lyx
