/**
 * \file ControlTabular.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlTabular.h"
#include "funcrequest.h"
#include "lyxrc.h"
#include "paragraph.h"
#include "insets/insettabular.h"


using std::string;

namespace lyx {
namespace frontend {

ControlTabular::ControlTabular(Dialog & parent)
	: Dialog::Controller(parent), active_cell_(-1)
{}


bool ControlTabular::initialiseParams(string const & data)
{
	Buffer & buffer = kernel().buffer();

	InsetTabular tmp(buffer);
	int cell = InsetTabularMailer::string2params(data, tmp);
	if (cell != -1) {
		params_.reset(new LyXTabular(tmp.tabular));
		active_cell_ = cell;
	}
	return true;
}


void ControlTabular::clearParams()
{
	params_.reset();
	active_cell_ = -1;
}


int ControlTabular::getActiveCell() const
{
	return active_cell_;
}


LyXTabular const & ControlTabular::tabular() const
{
	BOOST_ASSERT(params_.get());
	return *params_.get();
}


void ControlTabular::set(LyXTabular::Feature f, string const & arg)
{
	string const data = featureAsString(f) + ' ' + arg;
	kernel().dispatch(FuncRequest(LFUN_TABULAR_FEATURE, data));
}


bool ControlTabular::useMetricUnits() const
{
	return lyxrc.default_papersize > PAPER_EXECUTIVEPAPER;
}


void ControlTabular::toggleTopLine()
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(LyXTabular::M_TOGGLE_LINE_TOP);
	else
		set(LyXTabular::TOGGLE_LINE_TOP);
}


void ControlTabular::toggleBottomLine()
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(LyXTabular::M_TOGGLE_LINE_BOTTOM);
	else
		set(LyXTabular::TOGGLE_LINE_BOTTOM);
}


void ControlTabular::toggleLeftLine()
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(LyXTabular::M_TOGGLE_LINE_LEFT);
	else
		set(LyXTabular::TOGGLE_LINE_LEFT);
}


void ControlTabular::toggleRightLine()
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(LyXTabular::M_TOGGLE_LINE_RIGHT);
	else
		set(LyXTabular::TOGGLE_LINE_RIGHT);
}


void ControlTabular::setSpecial(string const & special)
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(LyXTabular::SET_SPECIAL_MULTI, special);
	else
		set(LyXTabular::SET_SPECIAL_COLUMN, special);
}


void ControlTabular::setWidth(string const & width)
{
	if (tabular().isMultiColumn(getActiveCell()))
		set(LyXTabular::SET_MPWIDTH, width);
	else
		set(LyXTabular::SET_PWIDTH, width);

	dialog().view().update();
}


void ControlTabular::toggleMultiColumn()
{
	set(LyXTabular::MULTICOLUMN);
	dialog().view().update();
}


void ControlTabular::rotateTabular(bool yes)
{
	if (yes)
		set(LyXTabular::SET_ROTATE_TABULAR);
	else
		set(LyXTabular::UNSET_ROTATE_TABULAR);
}


void ControlTabular::rotateCell(bool yes)
{
	if (yes)
		set(LyXTabular::SET_ROTATE_CELL);
	else
		set(LyXTabular::UNSET_ROTATE_CELL);
}


void ControlTabular::halign(ControlTabular::HALIGN h)
{
	LyXTabular::Feature num = LyXTabular::ALIGN_LEFT;
	LyXTabular::Feature multi_num = LyXTabular::M_ALIGN_LEFT;

	switch (h) {
		case LEFT:
			num = LyXTabular::ALIGN_LEFT;
			multi_num = LyXTabular::M_ALIGN_LEFT;
			break;
		case CENTER:
			num = LyXTabular::ALIGN_CENTER;
			multi_num = LyXTabular::M_ALIGN_CENTER;
			break;
		case RIGHT:
			num = LyXTabular::ALIGN_RIGHT;
			multi_num = LyXTabular::M_ALIGN_RIGHT;
			break;
		case BLOCK:
			num = LyXTabular::ALIGN_BLOCK;
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
	LyXTabular::Feature num = LyXTabular::VALIGN_MIDDLE;
	LyXTabular::Feature multi_num = LyXTabular::M_VALIGN_MIDDLE;

	switch (v) {
		case TOP:
			num = LyXTabular::VALIGN_TOP;
			multi_num = LyXTabular::M_VALIGN_TOP;
			break;
		case MIDDLE:
			num = LyXTabular::VALIGN_MIDDLE;
			multi_num = LyXTabular::M_VALIGN_MIDDLE;
			break;
		case BOTTOM:
			num = LyXTabular::VALIGN_BOTTOM;
			multi_num = LyXTabular::M_VALIGN_BOTTOM;
			break;
	}

	if (tabular().isMultiColumn(getActiveCell()))
		set(multi_num);
	else
		set(num);
}


void ControlTabular::longTabular(bool yes)
{
	if (yes)
		set(LyXTabular::SET_LONGTABULAR);
	else
		set(LyXTabular::UNSET_LONGTABULAR);
}

} // namespace frontend
} // namespace lyx
