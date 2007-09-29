// -*- C++ -*-
/**
 * \file bufferview_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFERVIEW_FUNCS_H
#define BUFFERVIEW_FUNCS_H

#include <string>

namespace lyx {

class Font;

namespace bv_funcs {

/// Set \param data using \param font and \param toggle. Return success.
bool font2string(Font const & font, bool toggle, std::string & data);

/// Set \param font and \param toggle using \param data. Return success.
bool string2font(std::string const & data, Font & font, bool & toggle);

/** Returns the current freefont, encoded as a std::string to be passed to the
 *  frontends.
 */
std::string const freefont2string();

} // namespace bv_funcs


} // namespace lyx

#endif
