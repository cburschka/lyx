// -*- C++ -*-
/**
 * \file ghelpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GHELPERS_H
#define GHELPERS_H

#include <string>

/** name is the name of the glade file, without path or extension.
 *  Eg, "aboutlyx", "tableCreate".
 */
std::string const findGladeFile(std::string const & name);

#endif // NOT GHELPERS_H
