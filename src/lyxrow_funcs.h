// -*- C++ -*-
/**
 * \file lyxrow_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXROW_FUNCS_H
#define LYXROW_FUNCS_H

#include "RowList.h"
#include "support/types.h"

class Paragraph;

bool isParEnd(Paragraph const & par, RowList::iterator rit);

lyx::pos_type lastPos(Paragraph const & par, RowList::iterator rit);

lyx::pos_type lastPrintablePos(Paragraph const & par, RowList::iterator rit);

int numberOfSeparators(Paragraph const & par, RowList::iterator rit);

int numberOfHfills(Paragraph const & par, RowList::iterator rit);

int numberOfLabelHfills(Paragraph const & par, RowList::iterator rit);

bool hfillExpansion(Paragraph const & par, RowList::iterator rit,
	lyx::pos_type pos);

#endif
