/**
 * \file GNote.C
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

#include "GNote.h"
#include "ControlNote.h"
#include "ghelpers.h"

#include "insets/insetnote.h"

#include <libglademm.h>

using std::string;

namespace lyx {
namespace frontend {

GNote::GNote(Dialog & parent)
	: GViewCB<ControlNote, GViewGladeB>(parent, _("Note Settings"), false)
{}


void GNote::doBuild()
{
	string const gladeName = findGladeFile("note");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * cancelbutton;
	xml_->get_widget("Close", cancelbutton);
	setCancel(cancelbutton);

	xml_->get_widget("LyXNote", lyxnoteradio_);
	xml_->get_widget("Comment", commentradio_);
	xml_->get_widget("GreyedOut", greyedoutradio_);

	lyxnoteradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GNote::apply));
	commentradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GNote::apply));
	greyedoutradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GNote::apply));

	bcview().addReadOnly(lyxnoteradio_);
	bcview().addReadOnly(commentradio_);
	bcview().addReadOnly(greyedoutradio_);
}


void GNote::update()
{
	applylock_ = true;

	bc().refreshReadOnly();

	switch (controller().params().type) {
	case InsetNoteParams::Note:
		lyxnoteradio_->set_active(true);
		break;
	case InsetNoteParams::Comment:
		commentradio_->set_active(true);
		break;
	case InsetNoteParams::Greyedout:
		greyedoutradio_->set_active(true);
		break;
	}

	applylock_ = false;
}


void GNote::apply()
{
	if (applylock_)
		return;

	InsetNoteParams::Type type;

	if (lyxnoteradio_->get_active())
		type = InsetNoteParams::Note;
	else if (greyedoutradio_->get_active())
		type = InsetNoteParams::Greyedout;
	else
		type = InsetNoteParams::Comment;

	controller().params().type = type;
	controller().dispatchParams();
}

} // namespace frontend
} // namespace lyx
