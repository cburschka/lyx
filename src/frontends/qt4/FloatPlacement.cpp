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

FloatPlacement::FloatPlacement(bool show_options, QWidget * parent)
	: QWidget(parent)
{
	setupUi(this);

	connect(topCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(bottomCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(pageCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(herepossiblyCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(heredefinitelyCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(ignoreCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(spanCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(sidewaysCB, SIGNAL(clicked()), this, SLOT(changedSlot()));

	spanCB->setVisible(show_options);
	sidewaysCB->setVisible(show_options);
}


docstring FloatPlacement::dialogToParams() const
{
	InsetFloatParams params;
	params.type = float_type_;
	params.placement = get(params.wide, params.sideways);
	return from_ascii(InsetFloat::params2string(params));
}


void FloatPlacement::useWide()
{
	spanCB->show();
}


void FloatPlacement::useSideways()
{
	sidewaysCB->show();
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
	heredefinitelyCB->setChecked(here_definitely);
	checkAllowed();
}


void FloatPlacement::paramsToDialog(Inset const * inset)
{
	InsetFloat const * fl = static_cast<InsetFloat const *>(inset);
	InsetFloatParams const & params = fl->params();

	float_type_ = params.type;
	floatType->setText(qt_(float_type_));

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


void FloatPlacement::on_defaultsCB_stateChanged(int state)
{
	checkAllowed();
	if (state == Qt::Checked)
		return;
	if (topCB->isChecked() || bottomCB->isChecked()
	   || pageCB->isChecked() || herepossiblyCB->isChecked()
	   || heredefinitelyCB->isChecked() || ignoreCB->isChecked())
		changed();
}


void FloatPlacement::changedSlot()
{
	checkAllowed();
	changed();
}


void FloatPlacement::checkAllowed()
{
	bool const defaults = defaultsCB->isChecked();
	bool const ignore = topCB->isChecked() || bottomCB->isChecked()
		      || pageCB->isChecked() || herepossiblyCB->isChecked();
	bool const heredefinitely = heredefinitelyCB->isChecked();

	// float or document dialog?
	if (spanCB->isVisible()) {
		bool const span = spanCB->isChecked();
		bool const sideways = sidewaysCB->isChecked();
		defaultsCB->setEnabled(!sideways);
		topCB->setEnabled(!sideways && !defaults && !heredefinitely);
		bottomCB->setEnabled(!sideways && !defaults && !span && !heredefinitely);
		pageCB->setEnabled(!sideways && !defaults && !heredefinitely);
		herepossiblyCB->setEnabled(!sideways && !defaults && !span && !heredefinitely);
		heredefinitelyCB->setEnabled(!sideways && !defaults && !span);
		ignoreCB->setEnabled(!sideways && !defaults && ignore && !heredefinitely);
		spanCB->setEnabled(!sideways || standardfloat_);
	} else {
		topCB->setEnabled(!defaults && !heredefinitely);
		bottomCB->setEnabled(!defaults && !heredefinitely);
		pageCB->setEnabled(!defaults && !heredefinitely);
		herepossiblyCB->setEnabled(!defaults && !heredefinitely);
		heredefinitelyCB->setEnabled(!defaults);
		ignoreCB->setEnabled(!defaults && ignore && !heredefinitely);
	}
}

} // namespace lyx

#include "moc_FloatPlacement.cpp"
