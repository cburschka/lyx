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
#include "LaTeX.h"
#include "buffer.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "debug.h"


using std::endl;


ControlErrorList::ErrorItem::ErrorItem(string const & error,
				       string const & description,
				       int par_id, int pos_start, int pos_end)
	: error(error), description(description), par_id(par_id),
	  pos_start(pos_start),  pos_end(pos_end)
{}


ControlErrorList::ControlErrorList(Dialog & d)
	: Dialog::Controller(d), current_(0)
{}


void ControlErrorList::clearParams()
{
	logfilename_.erase();
	clearErrors();
}


std::vector<ControlErrorList::ErrorItem> const &
ControlErrorList::ErrorList() const
{
	return ErrorList_;
}


int ControlErrorList::currentItem() const
{
	return current_;
}


bool ControlErrorList::initialiseParams(string const &)
{
	logfilename_ = kernel().buffer()->getLogName().second;
	clearErrors();
	fillErrors();
	current_ = 0;
	return true;
}


void ControlErrorList::clearErrors()
{
	ErrorList_.clear();
	current_ = 0;
}


void ControlErrorList::fillErrors()
{
	LaTeX latex("", logfilename_, "");
	TeXErrors terr;
	latex.scanLogFile(terr);

	Buffer * const buf = kernel().buffer();

	TeXErrors::Errors::const_iterator cit = terr.begin();
	TeXErrors::Errors::const_iterator end = terr.end();

	for (; cit != end; ++cit) {
		int par_id = -1;
		int posstart = -1;
		int const errorrow = cit->error_in_line;
		buf->texrow.getIdFromRow(errorrow, par_id, posstart);
		int posend = -1;
		buf->texrow.getIdFromRow(errorrow + 1, par_id, posend);
		ErrorList_.push_back(ErrorItem(cit->error_desc,
					       cit->error_text,
					       par_id, posstart, posend));
	}
}


string const & ControlErrorList::docName()
{
	return kernel().buffer()->fileName();
}


void ControlErrorList::goTo(int item)
{
	BufferView * const bv = kernel().bufferview();
	Buffer * const buf = kernel().buffer();

	current_ = item;

	ControlErrorList::ErrorItem const & err = ErrorList_[item];


	if (err.par_id == -1)
		return;

	ParagraphList::iterator pit = buf->getParFromID(err.par_id);

	if (pit == bv->text->ownerParagraphs().end()) {
		lyxerr << "par id not found" << endl;
		return;
	}

	int range = err.pos_end - err.pos_start;

	if (err.pos_end > pit->size() || range <= 0)
		range = pit->size() - err.pos_start;

	// Now make the selection.
	bv->insetUnlock();
	bv->toggleSelection();
	bv->text->clearSelection();
	bv->text->setCursor(pit, err.pos_start);
	bv->text->setSelectionRange(range);
	bv->toggleSelection(false);
	bv->fitCursor();
	bv->update(bv->text, BufferView::SELECT);
}
