// -*- C++ -*-
/**
 * \file forms_gettext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMS_GETTEXT_H
#define FORMS_GETTEXT_H

#include <string>

namespace lyx {
namespace frontend {

/// Extract shortcut from "<identifer>|<shortcut>" string
std::string const scex(std::string const &);

/// Extract identifier from "<identifer>|<shortcut>" string
std::string const idex(std::string const &);

} // namespace frontend
} // namespace lyx

#endif // FORMS_GETTEXT_H
