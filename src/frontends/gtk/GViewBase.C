/**
 * \file GViewBase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GViewBase.h"
#include "support/filetools.h"

using std::string;

namespace lyx {
namespace frontend {

GViewBase::GViewBase(Dialog & parent, string const & t, bool allowResize) :
	Dialog::View(parent, t), allowResize_(allowResize)
{
}


GViewBase::~GViewBase()
{
}


void GViewBase::hide()
{
	window()->hide();
}


void GViewBase::build()
{
	doBuild();
	string const iconName =
		support::LibFileSearch("images", "lyx", "xpm");
	if (!iconName.empty())
		window()->set_icon_from_file(iconName);
	window()->signal_delete_event().connect(
		sigc::mem_fun(*this, &GViewBase::onDeleteEvent));
	window()->set_title(Glib::locale_to_utf8(getTitle()));
}


void GViewBase::show()
{
	if (!window()) {
		build();
	}
	update();
	window()->show();
}


bool GViewBase::isVisible() const
{
	return window() && window()->is_visible();
}


GBC & GViewBase::bcview()
{
	return static_cast<GBC &>(dialog().bc().view());
}


void GViewBase::setCancel(Gtk::Button * cancel)
{
	bcview().setCancel(cancel);
	cancel->signal_clicked().connect(
		sigc::mem_fun(*this, &GViewBase::onCancel));
}


void GViewBase::setApply(Gtk::Button * apply)
{
	bcview().setApply(apply);
	apply->signal_clicked().connect(
		sigc::mem_fun(*this, &GViewBase::onApply));
}


void GViewBase::setOK(Gtk::Button * ok)
{
	bcview().setOK(ok);
	ok->signal_clicked().connect(
		sigc::mem_fun(*this, &GViewBase::onOK));
}


void GViewBase::setRestore(Gtk::Button * restore)
{
	bcview().setRestore(restore);
	restore->signal_clicked().connect(
		sigc::mem_fun(*this, &GViewBase::onRestore));
}


void GViewBase::setTitle(std::string const & title)
{
	Dialog::View::setTitle(title);
	window()->set_title(title);
}


bool GViewBase::readOnly() const
{
	return kernel().isBufferReadonly();
}


void GViewBase::onApply()
{
	dialog().ApplyButton();
}


void GViewBase::onOK()
{
	dialog().OKButton();
}


void GViewBase::onCancel()
{
	dialog().CancelButton();
}


void GViewBase::onRestore()
{
	dialog().RestoreButton();
}


bool GViewBase::onDeleteEvent(GdkEventAny *)
{
	dialog().CancelButton();
	return false;
}


GViewGladeB::GViewGladeB(Dialog & parent, string const & t, bool allowResize) :
	GViewBase(parent, t, allowResize)
{
}


Gtk::Window * GViewGladeB::window()
{
	Gtk::Window * win;
	if (!xml_)
		return 0;
	xml_->get_widget("dialog", win);
	return win;
}


Gtk::Window const * GViewGladeB::window() const
{
	Gtk::Window * win;
	if (!xml_)
		return 0;
	xml_->get_widget("dialog", win);
	return win;
}

} // namespace frontend
} // namespace lyx
