// -*- C++ -*-

#ifndef PARAGRAPH_LIST_H
#define PARAGRAPH_LIST_H

#include "paragraph.h"

#include <list>

struct ParagraphList : public std::list<Paragraph>
{
};

typedef std::pair<ParagraphList::iterator, int> PitPosPair;

#endif
