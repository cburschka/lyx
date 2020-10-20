/**
 * \file TocBuilder.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "TocBuilder.h"

#include "DocIterator.h"
#include "TocBackend.h"

#include "support/lassert.h"

using namespace std;


namespace lyx {



TocBuilder::TocBuilder(shared_ptr<Toc> toc)
	: toc_(toc ? toc : make_shared<Toc>()),
	  stack_()
{
	LATTEST(toc);
}

void TocBuilder::pushItem(DocIterator const & dit, docstring const & s,
                          bool output_active, bool is_captioned)
{
	toc_->push_back(TocItem(dit, stack_.size(), s, output_active));
	frame f = {
		toc_->size() - 1, //pos
		is_captioned, //is_captioned
	};
	stack_.push(f);
}

void TocBuilder::captionItem(DocIterator const & dit, docstring const & s,
                             bool output_active)
{
	// first show the float before moving to the caption
	docstring arg = "paragraph-goto " + dit.paragraphGotoArgument();
	if (!stack_.empty())
		arg = "paragraph-goto " +
			(*toc_)[stack_.top().pos].dit().paragraphGotoArgument() + ";" + arg;
	FuncRequest func(LFUN_COMMAND_SEQUENCE, arg);

	if (!stack_.empty() && !stack_.top().is_captioned) {
		// The float we entered has not yet been assigned a caption.
		// Assign the caption string to it.
		TocItem & captionable = (*toc_)[stack_.top().pos];
		captionable.str(s);
		captionable.setAction(func);
		stack_.top().is_captioned = true;
	} else {
		// This is a new entry.
		pop();
		// the dit is at the float's level, e.g. for the contextual menu of
		// outliner entries
		DocIterator captionable_dit = dit;
		captionable_dit.pop_back();
		pushItem(captionable_dit, s, output_active, true);
		(*toc_)[stack_.top().pos].setAction(func);
	}
}

void TocBuilder::argumentItem(docstring const & arg_str)
{
	if (stack_.empty() || arg_str.empty())
		return;
	TocItem & item = (*toc_)[stack_.top().pos];
	docstring const & str = item.str();
	string const & delim =
		(str.empty() || !stack_.top().is_captioned) ? "" :  ", ";
	item.str(str + from_ascii(delim) + arg_str);
	stack_.top().is_captioned = true;
}

void TocBuilder::pop()
{
	if (!stack_.empty())
		stack_.pop();
}


} // namespace lyx
