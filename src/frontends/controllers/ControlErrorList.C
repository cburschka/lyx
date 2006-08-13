/**
 * \file ControlErrorList.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlErrorList.h"
#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "gettext.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "pariterator.h"

#include "support/lstrings.h"

using lyx::support::bformat;

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

ControlErrorList::ControlErrorList(Dialog & d)
	: Dialog::Controller(d)
{}


void ControlErrorList::clearParams()
{}


ErrorList const & ControlErrorList::errorList() const
{
	return errorlist_;
}


bool ControlErrorList::initialiseParams(string const & error_type)
{
	Buffer * buf = kernel().bufferview()->buffer();
	string const title = bformat(_("%1$s Errors (%2$s)"),
		_(error_type), buf->fileName());
	errorlist_ = buf->errorList(error_type);
	name_ = title;
	return true;
}


string const & ControlErrorList::name()
{
	return name_;
}


void ControlErrorList::goTo(int item)
{
	ErrorItem const & err = errorlist_[item];

	if (err.par_id == -1)
		return;

	Buffer & buf = kernel().buffer();
	ParIterator pit = buf.getParFromID(err.par_id);

	if (pit == buf.par_iterator_end()) {
		lyxerr << "par id " << err.par_id << " not found" << endl;
		return;
	}

	// Now make the selection.
	// This should be implemented using an LFUN. (Angus)
	// if pos_end is 0, this means it is end-of-paragraph
	pos_type const end = err.pos_end ? std::min(err.pos_end, pit->size())
					 : pit->size();
	pos_type const start = std::min(err.pos_start, end);
	pos_type const range = end - start;
	DocIterator const dit = makeDocIterator(pit, start);
	kernel().bufferview()->putSelectionAt(dit, range, false);
	// If we used an LFUN, we would not need that
	kernel().bufferview()->update();
}

} // namespace frontend
} // namespace lyx
