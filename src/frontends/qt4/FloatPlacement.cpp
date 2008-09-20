/**
 * \file FloatPlacement.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FloatPlacement.h"
#include "qt_helpers.h"

#include "insets/InsetFloat.h"
#include "support/lstrings.h"

using namespace std;
using namespace lyx::support;


namespace lyx {

FloatPlacement::FloatPlacement(QWidget *)
{
	setupUi(this);

	connect(topCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(bottomCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(pageCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(herepossiblyCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(defaultsCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(defaultsCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(ignoreCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(pageCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(heredefinitelyCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(herepossiblyCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(bottomCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(topCB, SIGNAL(clicked()), this, SLOT(changedSlot()));

	spanCB->hide();
	sidewaysCB->hide();
}


void FloatPlacement::useWide()
{
	spanCB->show();
}


void FloatPlacement::useSideways()
{
	sidewaysCB->show();
}


void FloatPlacement::changedSlot()
{
	// emit signal
	changed();
}


void FloatPlacement::set(string const & placement)
{
	bool def_placement = false;
	bool top = false;
	bool bottom = false;
	bool page = false;
	bool here = false;
	bool force = false;
	bool here_definitely = false;

	if (placement.empty()) {
		def_placement = true;
	} else if (contains(placement, 'H')) {
		here_definitely = true;
	} else {
		if (contains(placement, '!')) {
			force = true;
		}
		if (contains(placement, 't')) {
			top = true;
		}
		if (contains(placement, 'b')) {
			bottom = true;
		}
		if (contains(placement, 'p')) {
			page = true;
		}
		if (contains(placement, 'h')) {
			here = true;
		}
	}

	defaultsCB->setChecked(def_placement);
	topCB->setChecked(top);
	bottomCB->setChecked(bottom);
	pageCB->setChecked(page);
	herepossiblyCB->setChecked(here);
	ignoreCB->setChecked(force);
	ignoreCB->setEnabled(top || bottom || page || here);
	heredefinitelyCB->setChecked(here_definitely);
	checkAllowed();
}


void FloatPlacement::set(lyx::InsetFloatParams const & params)
{
	set(params.placement);

	standardfloat_ = (params.type == "figure"
		|| params.type == "table");

	if (params.wide) {
		herepossiblyCB->setChecked(false);
		heredefinitelyCB->setChecked(false);
		bottomCB->setChecked(false);
	}

	spanCB->setChecked(params.wide);
	sidewaysCB->setChecked(params.sideways);
	// the package rotfloat only has *-versions for figure and table
	spanCB->setEnabled(!params.sideways || standardfloat_);
	checkAllowed();
}


string const FloatPlacement::get(bool & wide, bool & sideways) const
{
	wide = spanCB->isChecked();
	sideways = sidewaysCB->isChecked();

	return get();
}


string const FloatPlacement::get() const
{
	string placement;

	if (defaultsCB->isChecked())
		return placement;

	if (heredefinitelyCB->isChecked()) {
		placement += 'H';
	} else {
		if (ignoreCB->isChecked()) {
			placement += '!';
		}
		if (topCB->isChecked()) {
			placement += 't';
		}
		if (bottomCB->isChecked()) {
			placement += 'b';
		}
		if (pageCB->isChecked()) {
			placement += 'p';
		}
		if (herepossiblyCB->isChecked()) {
			placement += 'h';
		}
	}
	return placement;
}


void FloatPlacement::tbhpClicked()
{
	heredefinitelyCB->setChecked(false);
	checkAllowed();
}


void FloatPlacement::on_heredefinitelyCB_clicked()
{
	if (heredefinitelyCB->isChecked())
		ignoreCB->setEnabled(false);

	topCB->setChecked(false);
	bottomCB->setChecked(false);
	pageCB->setChecked(false);
	herepossiblyCB->setChecked(false);
	ignoreCB->setChecked(false);
}


void FloatPlacement::on_spanCB_clicked()
{
	checkAllowed();
	changed();

	if (!spanCB->isChecked())
		return;

	herepossiblyCB->setChecked(false);
	heredefinitelyCB->setChecked(false);
	bottomCB->setChecked(false);
}


void FloatPlacement::on_sidewaysCB_clicked()
{
	checkAllowed();
	changed();
}


void FloatPlacement::checkAllowed()
{
	bool const defaults = defaultsCB->isChecked();
	bool ignore = topCB->isChecked();
	ignore |= bottomCB->isChecked();
	ignore |= pageCB->isChecked();
	ignore |= herepossiblyCB->isChecked();

	// float or document dialog?
	if (spanCB->isVisible()) {
		bool const span = spanCB->isChecked();
		bool const sideways = sidewaysCB->isChecked();
		defaultsCB->setEnabled(!sideways);
		topCB->setEnabled(!sideways && !defaults);
		bottomCB->setEnabled(!sideways && !defaults && !span);
		pageCB->setEnabled(!sideways && !defaults);
		ignoreCB->setEnabled(!sideways && !defaults && ignore);
		herepossiblyCB->setEnabled(!sideways && !defaults && !span);
		heredefinitelyCB->setEnabled(!sideways && !defaults && !span);
		spanCB->setEnabled(!sideways || standardfloat_);
	} else {
		topCB->setEnabled(!defaults);
		bottomCB->setEnabled(!defaults);
		pageCB->setEnabled(!defaults);
		ignoreCB->setEnabled(!defaults && ignore);
		herepossiblyCB->setEnabled(!defaults);
		heredefinitelyCB->setEnabled(!defaults);
	}
}

} // namespace lyx

#include "FloatPlacement_moc.cpp"
