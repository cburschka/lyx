/**
 * \file insetbutton.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insetbutton.h"
#include "debug.h"
#include "dimension.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"
#include "support/LAssert.h"
#include "lyxfont.h"
#include "frontends/font_metrics.h"

using std::ostream;
using std::endl;


void InsetButton::dimension(BufferView * bv, LyXFont const &,
	Dimension & dim) const
{
	lyx::Assert(bv);

	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();

	string const s = getScreenLabel(bv->buffer());

	if (editable())
		font_metrics::buttonText(s, font, dim.wid, dim.asc, dim.des);
	else
		font_metrics::rectText(s, font, dim.wid, dim.asc, dim.des);

	dim.wid += 4;
}


void InsetButton::draw(BufferView * bv, LyXFont const &,
			int baseline, float & x) const
{
	lyx::Assert(bv);
	cache(bv);

	Painter & pain = bv->painter();
	// Draw it as a box with the LaTeX text
	LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::command).decSize();

	string const s = getScreenLabel(bv->buffer());

	if (editable()) {
		pain.buttonText(int(x) + 2, baseline, s, font);
	} else {
		pain.rectText(int(x) + 2, baseline, s, font,
			      LColor::commandbg, LColor::commandframe);
	}

	x += width(bv, font);
}


void InsetButton::cache(BufferView * bv) const
{
	view_ = bv->owner()->view();
}


#warning Shouldnt this really return a shared_ptr<BufferView>? (Lgb)
BufferView * InsetButton::view() const
{
	return view_.lock().get();
}


dispatch_result InsetButton::localDispatch(FuncRequest const & cmd)
{
	return Inset::localDispatch(cmd);
}
