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

bool openFileWrite(std::ofstream & ofs, std::string const & fname);
bool openFileWrite(lyx::odocfstream & ofs, std::string const & fname);

#endif
