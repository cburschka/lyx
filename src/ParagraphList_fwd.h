// -*- C++ -*-
/**
 * \file ParagraphList_fwd.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPH_LIST_FWD_H
#define PARAGRAPH_LIST_FWD_H

#include "paragraph.h"

#include <vector>

class ParagraphList : public std::vector<Paragraph>
{
public:
	///
	typedef std::vector<Paragraph> BaseType;
	///
	ParagraphList();
	///
	template <class Iter>
	ParagraphList(Iter beg, Iter end)
		: BaseType(beg, end)
	{}
};

#endif
