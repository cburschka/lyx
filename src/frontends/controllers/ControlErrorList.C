/**
 * \file ControlErrorList.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlErrorList.h"
#include "support/lstrings.h" // tostr
#include "errorlist.h"
#include "buffer.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "debug.h"


using std::endl;


ControlErrorList::ControlErrorList(Dialog & d)
	: Dialog::Controller(d)
{}


void ControlErrorList::clearParams()
{}


ErrorList const &
ControlErrorList::errorList() const
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
	BufferView * const bv = kernel().bufferview();
	Buffer * const buf = kernel().buffer();

	ErrorItem const & err = errorlist_[item];


	if (err.par_id == -1)
		return;

	ParIterator pit = buf->getParFromID(err.par_id);

	if (pit == buf->par_iterator_end()) {
		lyxerr << "par id not found" << endl;
		return;
	}

	int range = err.pos_end - err.pos_start;

	if (err.pos_end > (*pit)->size() || range <= 0)
		range = (*pit)->size() - err.pos_start;

	// Now make the selection.
	bv->insetUnlock();
	bv->toggleSelection();
	bv->text->clearSelection();
	bv->text->setCursor(*pit, err.pos_start);
	bv->text->setSelectionRange(range);
	bv->toggleSelection(false);
	bv->fitCursor();
	bv->update(bv->text, BufferView::SELECT);
}
