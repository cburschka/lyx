/**
 * \file GParagraph.C
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

#include "GParagraph.h"
#include "ghelpers.h"

#include "ControlParagraph.h"
#include "controllers/helper_funcs.h"

#include "ParagraphParameters.h"
#include "Spacing.h"
#include "support/lstrings.h"
#include "support/tostr.h"

using std::string;

namespace lyx {

namespace frontend {

namespace {

} // namespace anon


GParagraph::GParagraph(Dialog & parent)
	: GViewCB<ControlParagraph, GViewGladeB>(parent, _("Paragraph Settings"), false)
{}

void GParagraph::doBuild()
{
	string const gladeName = findGladeFile("paragraph");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	xml_->get_widget("LineSpacing", spacingspin_);
	xml_->get_widget("DefaultLineSpacing", defaultspacingcheck_);
	xml_->get_widget("MaxLabelWidth", maxlabelwidthentry_);
	xml_->get_widget("Indent", indentcheck_);
	xml_->get_widget("AlignBlock", blockradio_);
	xml_->get_widget("AlignLeft", leftradio_);
	xml_->get_widget("AlignRight", rightradio_);
	xml_->get_widget("AlignCenter", centerradio_);

	// Manage the Close button
	Gtk::Button * button;
	xml_->get_widget("Close", button);
	setCancel(button);

	// Make the main hbox sensitive to readonly
	Gtk::Widget * controlbox;
	xml_->get_widget("ControlBox", controlbox);
	bcview().addReadOnly(controlbox);

	spacingadj_ = spacingspin_->get_adjustment();

	defaultspacingcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GParagraph::onDefaultSpacingToggled));
	indentcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GParagraph::onIndentToggled));
	spacingadj_->signal_value_changed().connect(
		sigc::mem_fun(*this, &GParagraph::onSpacingChanged));
	maxlabelwidthentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GParagraph::onMaxLabelWidthChanged));

	blockradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GParagraph::onAlignToggled));
	leftradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GParagraph::onAlignToggled));
	rightradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GParagraph::onAlignToggled));
	centerradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GParagraph::onAlignToggled));
}


void GParagraph::update()
{
	// label width
	string const labelwidth = controller().params().labelWidthString();
	maxlabelwidthentry_->set_text(labelwidth);
	maxlabelwidthentry_->set_sensitive(
		labelwidth != _("Senseless with this layout!"));

	// alignment
	LyXAlignment const current_alignment = controller().params().align();
	switch (current_alignment) {
		case LYX_ALIGN_BLOCK:
			blockradio_->set_active(true);
			break;
		case LYX_ALIGN_LEFT:
			leftradio_->set_active(true);
			break;
		case LYX_ALIGN_RIGHT:
			rightradio_->set_active(true);
			break;
		case LYX_ALIGN_CENTER:
			centerradio_->set_active(true);
			break;
		default:
			// LYX_ALIGN_SPECIAL or so?  Don't ask, don't tell.
			centerradio_->set_active(false);
			blockradio_->set_active(false);
			rightradio_->set_active(false);
			leftradio_->set_active(false);
	}

	// Find out which alignments options are available
	LyXAlignment alignpos = controller().alignPossible();
	blockradio_->set_sensitive(bool(alignpos & LYX_ALIGN_BLOCK));
	centerradio_->set_sensitive(bool(alignpos & LYX_ALIGN_CENTER));
	leftradio_->set_sensitive(bool(alignpos & LYX_ALIGN_LEFT));
	rightradio_->set_sensitive(bool(alignpos & LYX_ALIGN_RIGHT));

	// We give the user a checkbox with an affirmative description, so
	// invert the setting
	indentcheck_->set_active(!controller().params().noindent());

	// linespacing
	Spacing const space = controller().params().spacing();

	// This emits the toggled signal, setting up sensitivities
	defaultspacingcheck_->set_active(
		space.getSpace() == Spacing::Default);

	spacingadj_->set_value(space.getValue());
}


void GParagraph::onDefaultSpacingToggled()
{
	if (defaultspacingcheck_->get_active()) {
		spacingspin_->set_sensitive(false);
		Spacing const spacing(Spacing::Default, spacingadj_->get_value());
		controller().params().spacing(spacing);
	}	else {
		spacingspin_->set_sensitive(true);
		Spacing const spacing(Spacing::Other, spacingadj_->get_value());
		controller().params().spacing(spacing);
	}
	controller().dispatchParams();
}


void GParagraph::onIndentToggled()
{
	controller().params().noindent(!indentcheck_->get_active());
	controller().dispatchParams();
}


void GParagraph::onSpacingChanged()
{
	Spacing const spacing(Spacing::Other, spacingadj_->get_value());
	controller().params().spacing(spacing);
	controller().dispatchParams();
}


void GParagraph::onMaxLabelWidthChanged()
{
	controller().params().labelWidthString(
		maxlabelwidthentry_->get_text());
	controller().dispatchParams();
}


void GParagraph::onAlignToggled()
{
	if (blockradio_->get_active())
		controller().params().align(LYX_ALIGN_BLOCK);
	else if (leftradio_->get_active())
		controller().params().align(LYX_ALIGN_LEFT);
	else if (rightradio_->get_active())
		controller().params().align(LYX_ALIGN_RIGHT);
	else if (centerradio_->get_active())
		controller().params().align(LYX_ALIGN_CENTER);

	controller().dispatchParams();
}


} // namespace frontend
} // namespace lyx
