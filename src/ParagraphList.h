// -*- C++ -*-
/**
 * \file ParagraphList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPH_LIST_H
#define PARAGRAPH_LIST_H

#include "Paragraph.h"

#include "support/RandomAccessList.h"


namespace lyx {

/// Container for all kind of Paragraphs used in LyX.
class ParagraphList : public RandomAccessList<Paragraph> {
public:
	///
	ParagraphList() {}
	///
	template<class InputIterator>
	ParagraphList(InputIterator first, InputIterator last)
		: RandomAccessList<Paragraph>(first, last)
	{}
};

} // namespace lyx

#endif
