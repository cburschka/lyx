// -*- C++ -*-
/**
 * \file forms_gettext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
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

/// Extract shortcut from "<ident>|<shortcut>" string
char const * scex(char const *);

/// Extract identifier from "<ident>|<shortcut>" string
char const * idex(char const *);

#endif // FORMS_GETTEXT_H
