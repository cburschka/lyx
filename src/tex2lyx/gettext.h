// -*- C++ -*-
/**
 * \file tex2lyx/gettext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */
#ifndef GETTEXT_H
#define GETTEXT_H

#include <string>


///
std::string const _(std::string const &);

/// for detecting static strings
#define N_(str) (str)

///
void locale_init();

#endif
