/**
 * \file GFloat.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GFloat.h"
#include "ControlFloat.h"

#include "ghelpers.h"

#include "insets/insetfloat.h"

#include "support/lstrings.h"

#include <libglademm.h>

using std::string;

namespace lyx {

using support::contains;

namespace frontend {

GFloat::GFloat(Dialog & parent)
	: GViewCB<ControlFloat, GViewGladeB>(parent, _("Float Settings"), false)
{}


void GFloat::doBuild()
{
	string const gladeName = findGladeFile("float");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * cancelbutton;
	xml_->get_widget("Close", cancelbutton);
	setCancel(cancelbutton);

	xml_->get_widget("Default", defaultradio_);
	xml_->get_widget("HereDefinitely", heredefinitelyradio_);
	xml_->get_widget("Alternative", alternativeradio_);
	xml_->get_widget("Top", topcheck_);
	xml_->get_widget("Bottom", bottomcheck_);
	xml_->get_widget("PageOfFloats", pageoffloatscheck_);
	xml_->get_widget("HereIfPossible", hereifpossiblecheck_);
	xml_->get_widget("IgnoreRules", ignorerulescheck_);
	xml_->get_widget("SpanColumns", spancolumnscheck_);
	xml_->get_widget("RotateSideways", rotatesidewayscheck_);

	defaultradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	heredefinitelyradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	alternativeradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	topcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	bottomcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	pageoffloatscheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	hereifpossiblecheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	ignorerulescheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	spancolumnscheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));
	rotatesidewayscheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GFloat::apply));

	bcview().addReadOnly(defaultradio_);
	bcview().addReadOnly(heredefinitelyradio_);
	bcview().addReadOnly(alternativeradio_);
	bcview().addReadOnly(topcheck_);
	bcview().addReadOnly(bottomcheck_);
	bcview().addReadOnly(pageoffloatscheck_);
	bcview().addReadOnly(hereifpossiblecheck_);
	bcview().addReadOnly(ignorerulescheck_);
	bcview().addReadOnly(spancolumnscheck_);
	bcview().addReadOnly(rotatesidewayscheck_);
}


void GFloat::update()
{
	applylock_ = true;

	bc().refreshReadOnly();

	string placement(controller().params().placement);
	bool const wide = controller().params().wide;
	bool const sideways = controller().params().sideways;

	bool const here_definitely = contains(placement, 'H');

	bool const top    = contains(placement, 't');
	bool const bottom = contains(placement, 'b');
	bool const page   = contains(placement, 'p');
	bool const here   = contains(placement, 'h');
	bool const force  = contains(placement, '!');
	bool const alternatives = top || bottom || page || (here && !wide);

	if (alternatives) {
		alternativeradio_->set_active(true);
	} else if (here_definitely) {
		heredefinitelyradio_->set_active(true);
	} else {
		defaultradio_->set_active(true);
	}
	ignorerulescheck_->set_active(force);
	topcheck_->set_active(top);
	bottomcheck_->set_active(bottom);
	pageoffloatscheck_->set_active(page);
	hereifpossiblecheck_->set_active(here);
	spancolumnscheck_->set_active(wide);
	rotatesidewayscheck_->set_active(sideways);

	updateSensitivity();

	applylock_ = false;
}


void GFloat::updateSensitivity()
{
	bool const wide = spancolumnscheck_->get_active();
	bool const sideways = rotatesidewayscheck_->get_active();
	bool const sideways_possible = (controller().params().type == "figure"
		|| controller().params().type == "table");
	bool const alternatives = alternativeradio_->get_active();
	bool const readonly = readOnly();

	heredefinitelyradio_->set_sensitive(!wide && !sideways && !readonly);
	ignorerulescheck_->set_sensitive(alternatives && !sideways && !readonly);
	topcheck_->set_sensitive(alternatives && !sideways && !readonly);
	bottomcheck_->set_sensitive(alternatives && !sideways && !readonly);
	pageoffloatscheck_->set_sensitive(alternatives && !sideways && !readonly);
	hereifpossiblecheck_->set_sensitive(alternatives && !wide && !sideways && !readonly);
	spancolumnscheck_->set_sensitive(!sideways && !readonly);
	defaultradio_->set_sensitive(!sideways && !readonly);
	alternativeradio_->set_sensitive(!sideways && !readonly);
	rotatesidewayscheck_->set_sensitive(sideways_possible && !readonly);
}


void GFloat::apply()
{
	if (applylock_)
		return;

	updateSensitivity();

	bool const wide = spancolumnscheck_->get_active();
	bool const sideways = rotatesidewayscheck_->get_active();

	int placementmode = 0; //default
	if (heredefinitelyradio_->get_active())
		placementmode = 1; // "Here, definitely"
	else if (alternativeradio_->get_active())
		placementmode = 2; // "Alternative"

	string placement;
	switch (placementmode) {
	case 2:
		// "Alternative"
		if (ignorerulescheck_->get_active()) {
			// Ignore internal LaTeX rules
			placement += '!';
		}
		if (topcheck_->get_active()) {
			// Top of page
			placement += 't';
		}
		if (bottomcheck_->get_active()) {
			// Bottom of page
			placement += 'b';
		}
		if (pageoffloatscheck_->get_active()) {
			// Page of floats
			placement += 'p';
		}
		// ignore if wide is selected
		if (!wide && hereifpossiblecheck_->get_active()) {
			// Here, if possible
			placement += 'h';
		}
		if (placement == "!") {
			// ignore placement if only force is selected.
			placement.erase();
		}
		break;

	case 1:
		// Here, definitely
		placement = "H";
		break;

	case 0:
		// default, do nothing.
		break;
	}

	controller().params().placement = placement;
	controller().params().wide = wide;
	controller().params().sideways = sideways;

	controller().dispatchParams();
}

} // namespace frontend
} // namespace lyx

