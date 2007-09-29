/**
 * \file ControlViewSource.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \author Bo Peng
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlViewSource.h"

#include "BufferView.h"
#include "Buffer.h"
#include "Cursor.h"
#include "gettext.h"
#include "Paragraph.h"
#include "TexRow.h"


using std::string;

namespace lyx {
namespace frontend {

ControlViewSource::ControlViewSource(Dialog & parent)
	: Controller(parent)
{}


bool ControlViewSource::initialiseParams(string const & /*source*/)
{
	return true;
}


docstring const ControlViewSource::updateContent(bool fullSource)
{
	// get the *top* level paragraphs that contain the cursor,
	// or the selected text
	pit_type par_begin;
	pit_type par_end;

	BufferView * view = bufferview();
	if (!view->cursor().selection()) {
		par_begin = view->cursor().bottom().pit();
		par_end = par_begin;
	} else {
		par_begin = view->cursor().selectionBegin().bottom().pit();
		par_end = view->cursor().selectionEnd().bottom().pit();
	}
	if (par_begin > par_end)
		std::swap(par_begin, par_end);
	odocstringstream ostr;
	view->buffer().getSourceCode(ostr, par_begin, par_end + 1, fullSource);
	return ostr.str();
}


std::pair<int, int> ControlViewSource::getRows() const
{
	BufferView const * view = bufferview();
	CursorSlice beg = view->cursor().selectionBegin().bottom();
	CursorSlice end = view->cursor().selectionEnd().bottom();

	int begrow = view->buffer().texrow().
		getRowFromIdPos(beg.paragraph().id(), beg.pos());
	int endrow = view->buffer().texrow().
		getRowFromIdPos(end.paragraph().id(), end.pos());
	int nextendrow = view->buffer().texrow().
		getRowFromIdPos(end.paragraph().id(), end.pos() + 1);
	return std::make_pair(begrow, endrow == nextendrow ? endrow : (nextendrow - 1));
}


void ControlViewSource::clearParams()
{
}


docstring const ControlViewSource::title() const
{
	switch (docType()) {
		case LATEX:
			return _("LaTeX Source");
		case DOCBOOK:
			return _("DocBook Source");
		case LITERATE:
			return _("Literate Source");
		default:
			BOOST_ASSERT(false);
			return docstring();
	}
}

} // namespace frontend
} // namespace lyx
