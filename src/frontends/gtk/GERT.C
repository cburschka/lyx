/**
 * \file GERT.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GERT.h"
#include "ControlERT.h"
#include "ghelpers.h"

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

GERT::GERT(Dialog & parent)
	: GViewCB<ControlERT, GViewGladeB>(parent, _("TeX Settings"), false)
{}


void GERT::doBuild()
{
	string const gladeName = findGladeFile("ERT");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * cancelbutton;
	xml_->get_widget("Close", cancelbutton);
	setCancel(cancelbutton);

	xml_->get_widget("Inline", inlineradio_);
	xml_->get_widget("Open", openradio_);
	xml_->get_widget("Collapsed", collapsedradio_);

	inlineradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GERT::apply));
	openradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GERT::apply));
	collapsedradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GERT::apply));

	bcview().addReadOnly(inlineradio_);
	bcview().addReadOnly(openradio_);
	bcview().addReadOnly(collapsedradio_);
}


void GERT::update()
{
	applylock_ = true;

	bc().refreshReadOnly();

	switch (controller().status()) {
	case InsetERT::Open:
		openradio_->set_active(true);
		break;
	case InsetERT::Collapsed:
		collapsedradio_->set_active(true);
		break;
	case InsetERT::Inlined:
		inlineradio_->set_active(true);
		break;
	}

	applylock_ = false;
}


void GERT::apply()
{
	if (applylock_)
		return;

	if (openradio_->get_active())
		controller().setStatus(InsetERT::Open);
	else if (collapsedradio_->get_active())
		controller().setStatus(InsetERT::Collapsed);
	else
		controller().setStatus(InsetERT::Inlined);

	controller().dispatchParams();
}

} // namespace frontend
} // namespace lyx
