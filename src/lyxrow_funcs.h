// -*- C++ -*-

#ifndef LYXROW_FUNCS_H
#define LYXROW_FUNCS_H

#include "RowList.h"
#include "support/types.h"

class LyXText;

bool isParEnd(LyXText const & lt, RowList::iterator rit);

lyx::pos_type lastPos(LyXText const & lt, RowList::iterator rit);

lyx::pos_type lastPrintablePos(LyXText const & lt, RowList::iterator rit);

int numberOfSeparators(LyXText const & lt, RowList::iterator rit);

int numberOfHfills(LyXText const & lt, RowList::iterator rit);

int numberOfLabelHfills(LyXText const & lt, RowList::iterator rit);

bool hfillExpansion(LyXText const & lt, RowList::iterator rit, lyx::pos_type pos);

#endif
