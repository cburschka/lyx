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

#include "support/types.h"


namespace lyx {

class Paragraph;
class Row;

bool hfillExpansion(Paragraph const & par, Row const & row,
	pos_type pos);


} // namespace lyx

#endif
