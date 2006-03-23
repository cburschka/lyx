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

#ifndef PARAGRAPH_LIST_H
#define PARAGRAPH_LIST_H

#include "paragraph.h"

#include "support/RandomAccessList.h"

/// Container for all kind of Paragraphs used in Lyx.
typedef RandomAccessList<Paragraph> ParagraphList;

#endif
