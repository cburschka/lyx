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

#include <list>
#include <utility>

class Paragraph;

typedef std::list<Paragraph> ParagraphList;

typedef std::pair<ParagraphList::iterator, int> PitPosPair;

#endif
