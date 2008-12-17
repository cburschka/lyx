// -*- C++ -*-
/**
 * \file output.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef OUTPUT_H
#define OUTPUT_H

#include "support/docstream.h"


namespace lyx {

namespace support { class FileName; }

bool openFileWrite(std::ofstream & ofs, support::FileName const & fname);
bool openFileWrite(ofdocstream & ofs, support::FileName const & fname);


} // namespace lyx

#endif
