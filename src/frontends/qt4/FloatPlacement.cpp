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

#include "Buffer.h"
#include "BufferParams.h"
#include "FloatList.h"
#include "TextClass.h"

#include "insets/InsetFloat.h"
#include "support/lstrings.h"

#include <map>

using namespace std;
using namespace lyx::support;


namespace lyx {

namespace frontend {

FloatPlacement::FloatPlacement(bool show_options, QWidget * parent)
	: InsetParamsWidget(parent), float_list_(0)
{
	setupUi(this);

	connect(floatTypeCO, SIGNAL(activated(int)), this, SLOT(changedSlot()));
	connect(topCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(bottomCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(pageCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(herepossiblyCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(heredefinitelyCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(ignoreCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(spanCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(sidewaysCB, SIGNAL(clicked()), this, SLOT(changedSlot()));

	floatTypeTitle->setVisible(show_options);
	floatTypeCO->setVisible(show_options);
	spanCB->setVisible(show_options);
	sidewaysCB->setVisible(show_options);
}


docstring FloatPlacement::dialogToParams() const
{
	InsetFloatParams params;
	params.type = fromqstr(floatTypeCO->itemData(floatTypeCO->currentIndex()).toString());
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


bool FloatPlacement::possiblePlacement(char const & p) const
{
	return !spanCB->isVisible() || contains(allowed_placement_, p);
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
	} else if (contains(placement, 'H') && possiblePlacement('H')) {
		here_definitely = true;
	} else {
		if (contains(placement, '!') && possiblePlacement('!')) {
			force = true;
		}
		if (contains(placement, 't') && possiblePlacement('t')) {
			top = true;
		}
		if (contains(placement, 'b') && possiblePlacement('b')) {
			bottom = true;
		}
		if (contains(placement, 'p') && possiblePlacement('p')) {
			page = true;
		}
		if (contains(placement, 'h') && possiblePlacement('h')) {
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


void FloatPlacement::initFloatTypeCO(FloatList const & floats)
{
	if (float_list_ == &floats)
		return;

	float_list_ = &floats;
	floatTypeCO->clear();
	FloatList::const_iterator it = floats.begin();
	FloatList::const_iterator const end = floats.end();
	for (; it != end; ++it) {
		floatTypeCO->addItem(qt_(it->second.name()),
				     toqstr(it->second.floattype()));
	}
}


void FloatPlacement::paramsToDialog(Inset const * inset)
{
	InsetFloat const * fl = static_cast<InsetFloat const *>(inset);
	InsetFloatParams const & params = fl->params();

	BufferParams const & bp = fl->buffer().params();
	initFloatTypeCO(bp.documentClass().floats());

	int const item = floatTypeCO->findData(toqstr(params.type));
	floatTypeCO->setCurrentIndex(item);

	allowed_placement_ =
		bp.documentClass().floats().allowedPlacement(params.type);

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


void FloatPlacement::checkAllowed() const
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
		topCB->setEnabled(!sideways && !defaults && !heredefinitely
				  && contains(allowed_placement_, 't'));
		bottomCB->setEnabled(!sideways && !defaults && !span && !heredefinitely
				     && contains(allowed_placement_, 'b'));
		pageCB->setEnabled(!sideways && !defaults && !heredefinitely
				   && contains(allowed_placement_, 'p'));
		herepossiblyCB->setEnabled(!sideways && !defaults && !span && !heredefinitely
					   && contains(allowed_placement_, 'h'));
		heredefinitelyCB->setEnabled(!sideways && !defaults && !span
					     && contains(allowed_placement_, 'H'));
		ignoreCB->setEnabled(!sideways && !defaults && ignore && !heredefinitely
				     && contains(allowed_placement_, '!'));
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


bool FloatPlacement::checkWidgets(bool readonly) const
{
	if (readonly) {
		floatTypeCO->setEnabled(false);
		defaultsCB->setEnabled(false);
		options->setEnabled(false);
		spanCB->setEnabled(false);
		sidewaysCB->setEnabled(false);
	} else
		checkAllowed();

	return InsetParamsWidget::checkWidgets();
}

} // namespace frontend
} // namespace lyx

#include "moc_FloatPlacement.cpp"
