/**
 * \file GSearch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GSearch.h"
#include "ControlSearch.h"
#include "ghelpers.h"

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

typedef GViewCB<ControlSearch, GViewGladeB> base_class;


GSearch::GSearch(Dialog & parent)
	: base_class(parent, _("Find and Replace"), false)
{}


void GSearch::doBuild()
{
	string const gladeName = findGladeFile("search");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * cancelbutton;
	xml_->get_widget("Cancel", cancelbutton);
	setCancel(cancelbutton);

	xml_->get_widget("FindNext", findnextbutton);
	xml_->get_widget("Replace", replacebutton);
	xml_->get_widget("ReplaceAll", replaceallbutton);
	xml_->get_widget("FindEntry", findentry);
	xml_->get_widget("ReplaceEntry", replaceentry);
	xml_->get_widget("CaseSensitive", casecheck);
	xml_->get_widget("MatchWord", matchwordcheck);
	xml_->get_widget("SearchBackwards", backwardscheck);

	findnextbutton->signal_clicked().connect(
		sigc::mem_fun(*this, &GSearch::onFindNext));
	replacebutton->signal_clicked().connect(
		sigc::mem_fun(*this, &GSearch::onReplace));
	replaceallbutton->signal_clicked().connect(
		sigc::mem_fun(*this, &GSearch::onReplaceAll));
	findentry->signal_changed().connect(
		sigc::mem_fun(*this,&GSearch::onFindEntryChanged));

	bcview().addReadOnly(replaceentry);
	bcview().addReadOnly(replacebutton);
	bcview().addReadOnly(replaceallbutton);
}


void GSearch::onFindNext()
{
	controller().find(findentry->get_text(),
	                  casecheck->get_active(),
	                  matchwordcheck->get_active(),
	                  !backwardscheck->get_active());
}


void GSearch::onReplace()
{
	controller().replace(findentry->get_text(),
	                     replaceentry->get_text(),
	                     casecheck->get_active(),
	                     matchwordcheck->get_active(),
	                     !backwardscheck->get_active(),
	                     false);
}


void GSearch::onReplaceAll()
{
	controller().replace(findentry->get_text(),
	                     replaceentry->get_text(),
	                     casecheck->get_active(),
	                     matchwordcheck->get_active(),
	                     !backwardscheck->get_active(),
	                     true);
}


void GSearch::onFindEntryChanged()
{
	if (findentry->get_text().empty()) {
		findnextbutton->set_sensitive(false);
		replacebutton->set_sensitive(false);
		replaceallbutton->set_sensitive(false);
	} else {
		findnextbutton->set_sensitive(true);
		replacebutton->set_sensitive(true);
		replaceallbutton->set_sensitive(true);
	}
}

} // namespace frontend
} // namespace lyx
