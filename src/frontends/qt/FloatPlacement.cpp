/**
 * \file FloatPlacement.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 * \author Jürgen Spitzmüller
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
	: InsetParamsWidget(parent), standardfloat_(true),
	  allows_wide_(true), allows_sideways_(true), float_list_(0)
{
	setupUi(this);

	connect(floatTypeCO, SIGNAL(activated(int)), this, SLOT(changedSlot()));
	connect(placementCO, SIGNAL(activated(int)), this, SLOT(changedSlot()));
	connect(topCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(bottomCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(pageCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(herepossiblyCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(heredefinitelyCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(ignoreCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(spanCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(sidewaysCB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(alignClassDefaultRB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(alignDocDefaultRB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(alignLeftRB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(alignCenterRB, SIGNAL(clicked()), this, SLOT(changedSlot()));
	connect(alignRightRB, SIGNAL(clicked()), this, SLOT(changedSlot()));

	floatTypeTitle->setVisible(show_options);
	floatTypeCO->setVisible(show_options);
	alignDocDefaultRB->setVisible(show_options);
	spanCB->setVisible(show_options);
	sidewaysCB->setVisible(show_options);
	optionsGB->setVisible(show_options);
	initFloatPlacementCO(show_options);
}


docstring FloatPlacement::dialogToParams() const
{
	InsetFloatParams params;
	params.type = fromqstr(floatTypeCO->itemData(floatTypeCO->currentIndex()).toString());
	params.wide = spanCB->isChecked();
	params.sideways = sidewaysCB->isChecked();
	params.alignment = getAlignment();
	params.placement = getPlacement();
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


void FloatPlacement::setPlacement(string const & placement)
{
	bool def_placement = false;
	bool doc_placement = false;
	bool top = false;
	bool bottom = false;
	bool page = false;
	bool here = false;
	bool force = false;
	bool here_definitely = false;

	if (placement.empty() || placement == "class")
		def_placement = true;
	else if (placement == "document")
		doc_placement = true;
	else if (contains(placement, 'H') && possiblePlacement('H')) {
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

	if (def_placement)
		placementCO->setCurrentIndex(placementCO->findData(toqstr("class")));
	else if (doc_placement)
		placementCO->setCurrentIndex(placementCO->findData(toqstr("document")));
	else
		placementCO->setCurrentIndex(placementCO->findData(toqstr("custom")));
	topCB->setChecked(top);
	bottomCB->setChecked(bottom);
	pageCB->setChecked(page);
	herepossiblyCB->setChecked(here);
	ignoreCB->setChecked(force);
	heredefinitelyCB->setChecked(here_definitely);
	checkAllowed();
}


void FloatPlacement::setAlignment(string const & alignment)
{
	if (alignment == "document")
		alignDocDefaultRB->setChecked(true);
	else if (alignment == "left")
		alignLeftRB->setChecked(true);
	else if (alignment == "center")
		alignCenterRB->setChecked(true);
	else if (alignment == "right")
		alignRightRB->setChecked(true);
	else
		alignClassDefaultRB->setChecked(true);
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


void FloatPlacement::initFloatPlacementCO(bool const local)
{
	placementCO->addItem(qt_("Class Default"), "class");
	if (local)
		placementCO->addItem(qt_("Document Default"), "document");
	placementCO->addItem(qt_("Custom"), "custom");
}


void FloatPlacement::paramsToDialog(Inset const * inset)
{
	InsetFloat const * fl = static_cast<InsetFloat const *>(inset);
	InsetFloatParams const & params = fl->params();

	BufferParams const & bp = fl->buffer().params();
	FloatList const & floats = bp.documentClass().floats();
	initFloatTypeCO(floats);

	int const item = floatTypeCO->findData(toqstr(params.type));
	floatTypeCO->setCurrentIndex(item);

	allowed_placement_ = floats.allowedPlacement(params.type);
	allows_sideways_ = floats.allowsSideways(params.type);
	allows_wide_ = floats.allowsWide(params.type);

	setPlacement(params.placement);
	setAlignment(params.alignment);

	standardfloat_ = (params.type == "figure"
		|| params.type == "table");

	if (params.wide) {
		herepossiblyCB->setChecked(false);
		heredefinitelyCB->setChecked(false);
		bottomCB->setChecked(false);
	}

	spanCB->setChecked(params.wide && allows_wide_);
	sidewaysCB->setChecked(params.sideways && allows_sideways_);

	checkAllowed();
}


string const FloatPlacement::getPlacement() const
{
	string placement;

	QString const data =
		placementCO->itemData(placementCO->currentIndex()).toString();
	if (data == "class")
		return "class";
	if (data == "document")
		return "document";

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


string const FloatPlacement::getAlignment() const
{
	if (alignDocDefaultRB->isChecked())
		return "document";
	if (alignLeftRB->isChecked())
		return "left";
	if (alignCenterRB->isChecked())
		return "center";
	if (alignRightRB->isChecked())
		return "right";
	return "class";
}


void FloatPlacement::on_placementCO_currentIndexChanged(QString const & text)
{
	checkAllowed();
	if (text != "custom")
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
	bool const defaults =
		placementCO->itemData(placementCO->currentIndex()).toString() != "custom";
	bool const ignore = topCB->isChecked() || bottomCB->isChecked()
		      || pageCB->isChecked() || herepossiblyCB->isChecked();
	bool const heredefinitely = heredefinitelyCB->isChecked();

	// float or document dialog?
	if (spanCB->isVisible()) {
		bool const span = spanCB->isChecked();
		bool const sideways = sidewaysCB->isChecked();
		topCB->setEnabled(!sideways && !defaults && !heredefinitely
				  && contains(allowed_placement_, 't'));
		bottomCB->setEnabled(!sideways && !defaults && !span && !heredefinitely
				     && contains(allowed_placement_, 'b'));
		pageCB->setEnabled(!sideways && !defaults && !heredefinitely
				   && contains(allowed_placement_, 'p'));
		if (!pageCB->isChecked())
			pageCB->setChecked(sideways && contains(allowed_placement_, 'p'));
		herepossiblyCB->setEnabled(!defaults && !span && !heredefinitely
					   && contains(allowed_placement_, 'h'));
		heredefinitelyCB->setEnabled(!defaults && !span
					     && contains(allowed_placement_, 'H'));
		ignoreCB->setEnabled(!defaults && ignore && !heredefinitely
				     && contains(allowed_placement_, '!'));
		// handle special case with sideways
		if ((!herepossiblyCB->isChecked() && sideways) || (span && sideways))
			ignoreCB->setEnabled(false);
		// when disabled the options must be unchecked to avoid strange LaTeX export
		// don't do it for pageCB because this case is handled with sideways
		if (ignoreCB->isChecked() && !ignoreCB->isEnabled())
			ignoreCB->setChecked(false);
		if (herepossiblyCB->isChecked() && !herepossiblyCB->isEnabled())
			herepossiblyCB->setChecked(false);
		if (topCB->isChecked() && !topCB->isEnabled())
			topCB->setChecked(false);
		if (bottomCB->isChecked() && !bottomCB->isEnabled())
			bottomCB->setChecked(false);
		spanCB->setEnabled(allows_wide_ && (!sideways || standardfloat_));
		sidewaysCB->setEnabled(allows_sideways_);
		placementCO->setEnabled(!(sideways && span));
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
		placementCO->setEnabled(false);
		aligmentGB->setEnabled(false);
		spanCB->setEnabled(false);
		sidewaysCB->setEnabled(false);
	} else
		checkAllowed();

	return InsetParamsWidget::checkWidgets();
}

} // namespace frontend
} // namespace lyx

#include "moc_FloatPlacement.cpp"
