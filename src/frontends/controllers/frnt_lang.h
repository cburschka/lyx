// -*- C++ -*-
/*
 * \file frnt_lang.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 */

#ifndef FRNT_LANG_H
#define FRNT_LANG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <utility>
#include <vector>

/** Ease the use of internationalised language strings in the dialogs.
 */
namespace frnt {
	///
	typedef std::pair<string, string> LanguagePair;
	/** If the caller is the character dialog, add "No change" and "Reset"
	 *  to the vector.
	 */
	std::vector<LanguagePair> const getLanguageData(bool character_dlg);

} // namespace frnt

#endif // FRNT_LANG_H
