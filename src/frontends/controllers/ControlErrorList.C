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

	lyx::pos_type const end = std::min(err.pos_end, pit->size());
	lyx::pos_type const start = std::min(err.pos_start, end);
	lyx::pos_type const range = end - start;

	// Now make the selection.
#ifdef WITH_WARNINGS
#warning FIXME (goto error)
#warning This should be implemented using an LFUN. (Angus)
#endif
#if 0
	PosIterator const pos(pit, start);
	kernel().bufferview()->putSelectionAt(pos, range, false);
#endif
}
