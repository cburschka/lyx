// -*- C++ -*-
/*
 * \file frnt_lang.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
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
	///
	std::vector<LanguagePair> const getLanguageData();

} // namespace frnt

#endif // FRNT_LANG_H
