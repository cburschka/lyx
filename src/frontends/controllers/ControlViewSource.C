/**
 * \file ControlViewSource.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlViewSource.h"
#include "gettext.h"
#include "support/types.h"
#include "BufferView.h"
#include "buffer.h"
#include "cursor.h"
#include <sstream>

using std::string;
using std::ostringstream;

namespace lyx {
namespace frontend {

ControlViewSource::ControlViewSource(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlViewSource::initialiseParams(string const & source)
{
	return true;
}

string const ControlViewSource::updateContent()
{	
	// get the *top* level paragraphs that contain the cursor, 
	// or the selected text
	lyx::pit_type par_begin;
	lyx::pit_type par_end;

	BufferView * view = kernel().bufferview();
	if (!view->cursor().selection()) {
		par_begin = view->cursor().bottom().pit();
		par_end = par_begin;
	} else {
		par_begin = view->cursor().selectionBegin().bottom().pit();
		par_end = view->cursor().selectionEnd().bottom().pit();
	}
	if (par_begin > par_end)
		std::swap(par_begin, par_end);
	ostringstream ostr;
	view->buffer()->getSourceCode(ostr, par_begin, par_end + 1);
	return ostr.str();
}


void ControlViewSource::clearParams()
{
}


string const ControlViewSource::title() const
{
	string source_type;
	
	Kernel::DocType doctype = kernel().docType();
	switch (doctype) {
	case Kernel::LATEX:
		source_type = "LaTeX";
		break;
	case Kernel::LINUXDOC:
		source_type = "LinuxDoc";
		break;
	case Kernel::DOCBOOK:
		source_type = "DocBook";
		break;
	case Kernel::LITERATE:
		source_type = "Literate";
	default:
		BOOST_ASSERT(false);
	}
	return _(source_type + " Source");
}

} // namespace frontend
} // namespace lyx
