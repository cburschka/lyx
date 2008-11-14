// -*- C++ -*-
/**
 * \file Intl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INTL_H
#define INTL_H

#include "Trans.h"


namespace lyx {

/**
 * This class is used for managing keymaps
 * for composing characters in LyX.
 */
class Intl {
public:
	/// which keymap is currently used ?
	enum Keymap {
		PRIMARY,
		SECONDARY
	};

	Intl();

	/// {en/dis}able the keymap
	void keyMapOn(bool on);

	/// set the primary language keymap
	void keyMapPrim();

	/// set the secondary language keymap
	void keyMapSec();

	/// turn on/off key mappings, status in keymapon
	void toggleKeyMap();

	/// initialize key mapper
	void initKeyMapper(bool on);

	// Get the Translation Manager
	inline TransManager & getTransManager() { return trans; }

	/// using primary or secondary keymap ?
	Keymap keymap;

private:
	/// is key mapping enabled ?
	bool keymapon;
	/// the primary language keymap
	std::string & prim_lang;
	/// the secondary language keymap
	std::string & sec_lang;
	/// the translation manager
	TransManager trans;
};

} // namespace lyx

#endif /* INTL_H */
