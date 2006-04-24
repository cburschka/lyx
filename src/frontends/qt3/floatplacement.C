/**
 * \file floatplacement.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "floatplacement.h"
#include "qt_helpers.h"

#include "insets/insetfloat.h"
#include "support/lstrings.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qgroupbox.h>

using lyx::support::contains;

using std::string;


// FIXME: set disabled doesn't work properly
// should be fixed now (jspitzm)
FloatPlacement::FloatPlacement(QWidget * parent, char * name)
	: QWidget(parent, name)
{
	QHBoxLayout * toplayout = new QHBoxLayout(this, 11, 6);
	layout = new QVBoxLayout(0, 0, 6);
	QGroupBox * options = new QGroupBox(qt_("Advanced Placement Options"), this);

	defaultsCB = new QCheckBox(qt_("Use &default placement"), this);
	topCB = new QCheckBox(qt_("&Top of page"), options);
	bottomCB = new QCheckBox(qt_("&Bottom of page"), options);
	pageCB = new QCheckBox(qt_("&Page of floats"), options);
	herepossiblyCB = new QCheckBox(qt_("&Here if possible"), options);
	heredefinitelyCB = new QCheckBox(qt_("Here definitely"), options);
	ignoreCB = new QCheckBox(qt_("&Ignore LaTeX rules"), options);
	spanCB = 0;
	sidewaysCB = 0;

	layout->addWidget(defaultsCB);

	QVBoxLayout * optlay = new QVBoxLayout(options, 10, 6);
	optlay->addSpacing(6);
	optlay->addWidget(topCB);
	optlay->addWidget(bottomCB);
	optlay->addWidget(pageCB);
	optlay->addWidget(herepossiblyCB);
	optlay->addWidget(heredefinitelyCB);
	optlay->addWidget(ignoreCB);

	layout->addWidget(options);

	toplayout->addLayout(layout);

	connect(defaultsCB, SIGNAL(toggled(bool)), options, SLOT(setDisabled(bool)));

	connect(heredefinitelyCB, SIGNAL(clicked()), this, SLOT(heredefinitelyClicked()));
	connect(topCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(bottomCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(pageCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(herepossiblyCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));
	connect(defaultsCB, SIGNAL(clicked()), this, SLOT(tbhpClicked()));

	connect(defaultsCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
	connect(ignoreCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
	connect(pageCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
	connect(heredefinitelyCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
	connect(herepossiblyCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
	connect(bottomCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
	connect(topCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
}


void FloatPlacement::useWide()
{
	spanCB = new QCheckBox(qt_("&Span columns"), this);
	layout->addWidget(spanCB);
	setTabOrder(ignoreCB, spanCB);
	connect(spanCB, SIGNAL(clicked()), this, SLOT(spanClicked()));
	connect(spanCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
}


void FloatPlacement::useSideways()
{
	sidewaysCB = new QCheckBox(qt_("&Rotate sideways"), this);
	layout->addWidget(sidewaysCB);
	setTabOrder(spanCB, sidewaysCB);
	connect(sidewaysCB, SIGNAL(clicked()), this, SLOT(sidewaysClicked()));
	connect(sidewaysCB, SIGNAL(toggled(bool)), this, SLOT(changedSlot()));
}


void FloatPlacement::changedSlot()
{
	emit changed();
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


void FloatPlacement::set(InsetFloatParams const & params)
{
	set(params.placement);

	if (params.wide) {
		herepossiblyCB->setChecked(false);
		heredefinitelyCB->setChecked(false);
		bottomCB->setChecked(false);
	}

	spanCB->setChecked(params.wide);
	sidewaysCB->setChecked(params.sideways);
	sidewaysCB->setEnabled(params.type == "figure"
		|| params.type == "table");
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


void FloatPlacement::heredefinitelyClicked()
{
	if (heredefinitelyCB->isChecked())
		ignoreCB->setEnabled(false);

	topCB->setChecked(false);
	bottomCB->setChecked(false);
	pageCB->setChecked(false);
	herepossiblyCB->setChecked(false);
	ignoreCB->setChecked(false);
}


void FloatPlacement::spanClicked()
{
	checkAllowed();

	if (!spanCB->isChecked())
		return;

	herepossiblyCB->setChecked(false);
	heredefinitelyCB->setChecked(false);
	bottomCB->setChecked(false);
}


void FloatPlacement::sidewaysClicked()
{
	checkAllowed();
}


void FloatPlacement::checkAllowed()
{
	bool const defaults(defaultsCB->isChecked());
	bool ignore(topCB->isChecked());
	ignore |= bottomCB->isChecked();
	ignore |= pageCB->isChecked();
	ignore |= herepossiblyCB->isChecked();

	// float or document dialog?
	if (spanCB != 0) {
		bool const span(spanCB->isChecked());
		bool const sideways(sidewaysCB->isChecked());
		defaultsCB->setEnabled(!sideways);
		topCB->setEnabled(!sideways && !defaults);
		bottomCB->setEnabled(!sideways && !defaults && !span);
		pageCB->setEnabled(!sideways && !defaults);
		ignoreCB->setEnabled(!sideways && !defaults && ignore);
		herepossiblyCB->setEnabled(!sideways && !defaults && !span);
		heredefinitelyCB->setEnabled(!sideways && !defaults && !span);
		spanCB->setEnabled(!sideways);
	} else {
		topCB->setEnabled(!defaults);
		bottomCB->setEnabled(!defaults);
		pageCB->setEnabled(!defaults);
		ignoreCB->setEnabled(!defaults && ignore);
		herepossiblyCB->setEnabled(!defaults);
		heredefinitelyCB->setEnabled(!defaults);
	}
}
