// -*- C++ -*-
/**
 * \file ParagraphList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPH_LIST_H
#define PARAGRAPH_LIST_H

#include "paragraph.h"

#include <list>

struct ParagraphList : public std::list<Paragraph>
{
};

typedef std::pair<ParagraphList::iterator, int> PitPosPair;

#endif
