// -*- C++ -*-
/**
 * \file forms_gettext.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, leeming@lyx.org
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
