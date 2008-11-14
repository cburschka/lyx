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

#include <config.h>

#include "Intl.h"
#include "LyXRC.h"

#include "support/debug.h"

namespace lyx {


Intl::Intl()
	: keymap(Intl::PRIMARY), keymapon(lyxrc.use_kbmap),
	prim_lang(lyxrc.primary_kbmap), sec_lang(lyxrc.secondary_kbmap)
{
}


void Intl::keyMapOn(bool on)
{
	keymapon = on;

	if (on) {
		if (keymap == PRIMARY)
			keyMapPrim();
		else
			keyMapSec();
	} else {
		trans.disableKeymap();
	}
}


void Intl::toggleKeyMap()
{
	if (keymapon && (keymap == PRIMARY))
		keyMapSec();
	else if (keymapon)
		keyMapOn(false);
	else
		keyMapPrim();
}


void Intl::keyMapPrim()
{
	if (!trans.setPrimary(prim_lang))
		trans.enablePrimary();

	keymapon = true;
	keymap = PRIMARY;
}


void Intl::keyMapSec()
{
	if (!trans.setSecondary(sec_lang))
		trans.enableSecondary();

	keymapon = true;
	keymap = SECONDARY;
}


void Intl::initKeyMapper(bool on)
{
	LYXERR(Debug::INIT, "Initializing key mappings...");

	if (trans.setPrimary(prim_lang) == -1)
		prim_lang.erase();
	if (trans.setSecondary(sec_lang) == -1)
		sec_lang.erase();

	if (prim_lang.empty() && sec_lang.empty())
		keymapon = false;
	else
		keymapon = on;

	keyMapOn(keymapon);

	if (keymapon)
		keyMapPrim();
}


} // namespace lyx
