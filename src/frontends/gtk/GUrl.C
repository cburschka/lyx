/**
 * \file GUrl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>

#include <libglademm.h>

#include "ControlCommand.h"
#include "GUrl.h"
#include "ghelpers.h"
#include "support/lstrings.h"

using std::string;

namespace lyx {
namespace frontend {

GUrl::GUrl(Dialog & parent)
	: GViewCB<ControlCommand, GViewGladeB>(parent, _("URL"))
{
}


void GUrl::doBuild()
{
	string const gladeName = findGladeFile("url");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	Gtk::Button * restore;
	Gtk::Button * ok;
	Gtk::Button * apply;
	Gtk::Button * cancel;
	xml_->get_widget("Url", url_);
	xml_->get_widget("Name", name_);
	xml_->get_widget("Html", htmlType_);
	xml_->get_widget("Restore", restore);
	xml_->get_widget("Ok", ok);
	xml_->get_widget("Apply", apply);
	xml_->get_widget("Cancel", cancel);
	setOK(ok);
	setCancel(cancel);
	setApply(apply);
	setRestore(restore);
	bcview().addReadOnly(name_);
	bcview().addReadOnly(url_);
	bcview().addReadOnly(htmlType_);

	url_->signal_changed().connect(
		SigC::slot(*this, &GUrl::onEntryChanged));
	name_->signal_changed().connect(
		SigC::slot(*this, &GUrl::onEntryChanged));
}


void GUrl::onEntryChanged()
{
	bc().valid(!url_->get_text().empty() || !name_->get_text().empty());
}


void GUrl::update()
{
	url_->set_text(Glib::locale_to_utf8(
			       controller().params().getContents()));
	name_->set_text(Glib::locale_to_utf8(
				controller().params().getOptions()));
	if (controller().params().getCmdName() == "url")
		htmlType_->set_active(false);
	else
		htmlType_->set_active();
}


void GUrl::apply()
{
	controller().params().setContents(
		Glib::locale_to_utf8(url_->get_text()));
	controller().params().setOptions(
		Glib::locale_to_utf8(name_->get_text()));
	if (htmlType_->get_active())
		controller().params().setCmdName("htmlurl");
	else
		controller().params().setCmdName("url");
}

} // namespace frontend
} // namespace lyx
