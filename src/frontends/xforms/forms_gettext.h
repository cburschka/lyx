// -*- C++ -*-
/**
 * \file forms_gettext.h
 * Read the file COPYING
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMS_GETTEXT_H
#define FORMS_GETTEXT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "forms_fwd.h"

/// Extract shortcut from <ident>|<shortcut> string
char const * flyx_shortcut_extract(char const * sc);
/// Shortcut for flyx_shortcut_extract
#define scex flyx_shortcut_extract

/// Extract shortcut from <ident>|<shortcut> string
char const * flyx_ident_extract(char const * sc);
/// Shortcut for flyx_ident_extract
#define idex flyx_ident_extract

#endif // FORMS_GETTEXT_H
