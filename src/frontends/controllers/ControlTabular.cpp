/**
 * \file ControlTabular.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferView.h"
#include "ControlTabular.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "LyXRC.h"
#include "insets/InsetTabular.h"


using std::string;

namespace lyx {
namespace frontend {

ControlTabular::ControlTabular(Dialog & parent)
	: Controller(parent), active_cell_(Tabular::npos)
{}


bool ControlTabular::initialiseParams(string const & data)
{
	// try to get the current cell
	BufferView const * const bv = bufferview();
	InsetTabular const * current_inset = 0;
	if (bv) {
		Cursor const & cur = bv->cursor();
		// get the innermost tabular inset;
		// assume that it is "ours"
		for (int i = cur.depth() - 1; i >= 0; --i)
			if (cur[i].inset().lyxCode() == Inset::TABULAR_CODE) {
				current_inset = static_cast<InsetTabular const *>(&cur[i].inset());
				active_cell_ = cur[i].idx();
				break;
			}
	}

	if (current_inset && data.empty()) {
		params_.reset(new Tabular(current_inset->tabular));
		return true;
	}

	InsetTabular tmp(buffer());
	InsetTabularMailer::string2params(data, tmp);
	params_.reset(new Tabular(tmp.tabular));
	return true;
}


void ControlTabular::clearParams()
{
	params_.reset();
	active_cell_ = Tabular::npos;
}


Tabular::idx_type ControlTabular::getActiveCell() const
{
	return active_cell_;
}


Tabular const & ControlTabular::tabular() const
{
	BOOST_ASSERT(params_.get());
	return *params_.get();
}


void ControlTabular::set(Tabular::Feature f, string const & arg)
{
	string const data = featureAsString(f) + ' ' + arg;
	dispatch(FuncRequest(getLfun(), data));
}


bool ControlTabular::useMetricUnits() const
{
	return lyxrc.default_papersize > PAPER_USEXECUTIVE;
}


void ControlTabular::toggleTopLine()
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(Tabular::M_TOGGLE_LINE_TOP);
	else
		set(Tabular::TOGGLE_LINE_TOP);
}


void ControlTabular::toggleBottomLine()
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(Tabular::M_TOGGLE_LINE_BOTTOM);
	else
		set(Tabular::TOGGLE_LINE_BOTTOM);
}


void ControlTabular::toggleLeftLine()
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(Tabular::M_TOGGLE_LINE_LEFT);
	else
		set(Tabular::TOGGLE_LINE_LEFT);
}


void ControlTabular::toggleRightLine()
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(Tabular::M_TOGGLE_LINE_RIGHT);
	else
		set(Tabular::TOGGLE_LINE_RIGHT);
}


void ControlTabular::setSpecial(string const & special)
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(Tabular::SET_SPECIAL_MULTI, special);
	else
		set(Tabular::SET_SPECIAL_COLUMN, special);
}


void ControlTabular::setWidth(string const & width)
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(Tabular::SET_MPWIDTH, width);
	else
		set(Tabular::SET_PWIDTH, width);

	dialog().updateView();
}


void ControlTabular::toggleMultiColumn()
{
	set(Tabular::MULTICOLUMN);
	dialog().updateView();
}


void ControlTabular::rotateTabular(bool yes)
{
	if (yes)
		set(Tabular::SET_ROTATE_TABULAR);
	else
		set(Tabular::UNSET_ROTATE_TABULAR);
}


void ControlTabular::rotateCell(bool yes)
{
	if (yes)
		set(Tabular::SET_ROTATE_CELL);
	else
		set(Tabular::UNSET_ROTATE_CELL);
}


void ControlTabular::halign(ControlTabular::HALIGN h)
{
	Tabular::Feature num = Tabular::ALIGN_LEFT;
	Tabular::Feature multi_num = Tabular::M_ALIGN_LEFT;

	switch (h) {
		case LEFT:
			num = Tabular::ALIGN_LEFT;
			multi_num = Tabular::M_ALIGN_LEFT;
			break;
		case CENTER:
			num = Tabular::ALIGN_CENTER;
			multi_num = Tabular::M_ALIGN_CENTER;
			break;
		case RIGHT:
			num = Tabular::ALIGN_RIGHT;
			multi_num = Tabular::M_ALIGN_RIGHT;
			break;
		case BLOCK:
			num = Tabular::ALIGN_BLOCK;
			//multi_num: no equivalent
			break;
	}

	if (tabular().isMultiColumn(getActiveCell()))
		set(multi_num);
	else
		set(num);
}


void ControlTabular::valign(ControlTabular::VALIGN v)
{
	Tabular::Feature num = Tabular::VALIGN_MIDDLE;
	Tabular::Feature multi_num = Tabular::M_VALIGN_MIDDLE;

	switch (v) {
		case TOP:
			num = Tabular::VALIGN_TOP;
			multi_num = Tabular::M_VALIGN_TOP;
			break;
		case MIDDLE:
			num = Tabular::VALIGN_MIDDLE;
			multi_num = Tabular::M_VALIGN_MIDDLE;
			break;
		case BOTTOM:
			num = Tabular::VALIGN_BOTTOM;
			multi_num = Tabular::M_VALIGN_BOTTOM;
			break;
	}

	if (tabular().isMultiColumn(getActiveCell()))
		set(multi_num);
	else
		set(num);
}


void ControlTabular::booktabs(bool yes)
{
	if (yes)
		set(Tabular::SET_BOOKTABS);
	else
		set(Tabular::UNSET_BOOKTABS);
}


void ControlTabular::longTabular(bool yes)
{
	if (yes)
		set(Tabular::SET_LONGTABULAR);
	else
		set(Tabular::UNSET_LONGTABULAR);
}

} // namespace frontend
} // namespace lyx
