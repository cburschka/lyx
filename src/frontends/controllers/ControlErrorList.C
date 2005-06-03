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
#include "lyxtext.h"
#include "paragraph.h"
#include "pariterator.h"

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


bool ControlErrorList::initialiseParams(string const & name)
{
	errorlist_ = kernel().bufferview()->getErrorList();
	name_ = name;
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
		lyxerr << "par id not found" << endl;
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
