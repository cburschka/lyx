/**
 * \file GChanges.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include "GChanges.h"
#include "ControlChanges.h"

#include "ghelpers.h"

using std::string;

namespace lyx {
namespace frontend {


GChanges::GChanges(Dialog & parent)
	: GViewCB<ControlChanges, GViewGladeB>(parent, _("Merge Changes"), false)
{}


void GChanges::doBuild()
{
	string const gladeName = findGladeFile("changes");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	xml_->get_widget("Message", messagelabel_);

	Gtk::Button * closebutton;
	xml_->get_widget("Close", closebutton);
	setCancel(closebutton);

	xml_->get_widget("Accept", acceptbutton_);
	bcview().addReadOnly(acceptbutton_);
	acceptbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GChanges::onAccept));

	xml_->get_widget("Reject", rejectbutton_);
	bcview().addReadOnly(rejectbutton_);
	rejectbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GChanges::onReject));

	xml_->get_widget("Next", nextbutton_);
	nextbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GChanges::onNext));
}


void GChanges::update()
{
	onNext();
}


void GChanges::onAccept()
{
	if (controller().accept()) {
		promptChange();
	} else {
		promptDismiss();
	}
}


void GChanges::onReject()
{
	if (controller().reject()) {
		promptChange();
	} else {
		promptDismiss();
	}
}


void GChanges::onNext()
{
	if (controller().find()) {
		promptChange();
	} else {
		promptDismiss();
	}
}


void GChanges::promptChange()
{
	string const header = _("Accept highlighted change?");
	string author = controller().getChangeAuthor();
	string date = controller().getChangeDate();
	if(author.empty())
		author = _("unknown author");
	if(date.empty())
		date = _("unknown date");

	messagelabel_->set_markup("<big><b>" + header +
	                        "</b></big>\n\nChanged by <b>" + author
	                        + "</b> on <b>" + date + "</b>");

	acceptbutton_->set_sensitive(true && !readOnly());
	rejectbutton_->set_sensitive(true && !readOnly());
	nextbutton_->set_sensitive(true);
}


void GChanges::promptDismiss()
{
	string const header = _("Done merging changes");

	messagelabel_->set_markup("<big><b>" + header +
	                        "</b></big>");

	// Disable all buttons but close.
	acceptbutton_->set_sensitive(false);
	rejectbutton_->set_sensitive(false);
	nextbutton_->set_sensitive(false);
}


} // namespace frontend
} // namespace lyx
