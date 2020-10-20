// -*- C++ -*-
/**
 * \file TocBuilder.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOC_BUILDER_H
#define TOC_BUILDER_H

#include "Toc.h"

#include "support/strfwd.h"

#include <stack>


namespace lyx {

class DocIterator;

/// Caption-enabled TOC builders
class TocBuilder
{
public:
	TocBuilder(std::shared_ptr<Toc> toc);
	/// Open a level.
	/// When entering a float or flex or paragraph (with AddToToc)
	void pushItem(DocIterator const & dit, docstring const & s,
	              bool output_active, bool is_captioned = false);
	/// Edit entry at current level. Add new entry if already captioned.
	/// When encountering a float caption
	void captionItem(DocIterator const & dit, docstring const & s,
	                 bool output_active);
	/// Edit entry at current level (always).
	/// When encountering an argument (with isTocCaption) for flex or paragraph
	void argumentItem(docstring const & arg_str);
	/// Close a level.
	/// When exiting a float or flex or paragraph
	void pop();
private:
	TocBuilder(){}
	///
	struct frame {
		Toc::size_type pos;
		bool is_captioned;
	};
	///
	std::shared_ptr<Toc> const toc_;
	///
	std::stack<frame> stack_;
};


} // namespace lyx

#endif // TOC_BUILDER_H
