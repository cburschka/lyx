// -*- C++ -*-
/**
 *  \file GraphicsSupport.h
 *  Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GRAPHICSSUPPORT_H
#define GRAPHICSSUPPORT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "support/types.h"
#include <list>

class BufferView;
class Inset;
class Paragraph;

/** A Paragraph * together with delimiters for the start and end positions
    of visibility.
 */
struct VisibleParagraph {
	///
	VisibleParagraph() : par(0), start(0), end(0) {}
	///
	VisibleParagraph(Paragraph * p, lyx::pos_type s, lyx::pos_type e)
		: par(p), start(s), end(e) {}
	///
	Paragraph * par;
	///
	lyx::pos_type start;
	///
	lyx::pos_type end;
};


/// Returns a list of all Paragraphs currently visible in bv.
std::list<VisibleParagraph> const getVisibleParagraphs(BufferView const & bv);

/** Given this data, check whether inset lies within it and is, therefore,
 *  visible.
 */
bool isInsetVisible(Inset const & inset, std::list<VisibleParagraph> const &);

#endif // GRAPHICSSUPPORT_H
