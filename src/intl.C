/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

/*
 *	International support for LyX
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "intl.h"
#include "debug.h"
#include "lyxrc.h"

#include "support/lstrings.h"
#include "language.h"

using std::endl;


Intl::Intl()
	: keymap(Intl::PRIMARY), keymapon(lyxrc.use_kbmap),
	prim_lang(lyxrc.primary_kbmap), sec_lang(lyxrc.secondary_kbmap)
{
}


void Intl::KeyMapOn(bool on)
{
	keymapon = on;

	if (on) {
		if (keymap == PRIMARY)
			KeyMapPrim();
		else
			KeyMapSec();
	} else
		trans.DisableKeymap();
}


void Intl::ToggleKeyMap()
{
	if (keymapon && (keymap == PRIMARY)) {
		KeyMapSec();
	} else if (keymapon) {
		KeyMapOn(false);
	} else	
		KeyMapPrim();
}


void Intl::KeyMapPrim()
{
	if (!trans.SetPrimary(prim_lang))
		trans.EnablePrimary();

	keymapon = true;
	keymap = PRIMARY;
}


void Intl::KeyMapSec()
{
	if (!trans.SetSecondary(sec_lang))
		trans.EnableSecondary();

	keymapon = true;
	keymap = SECONDARY;
}


void Intl::InitKeyMapper(bool on)
{
	lyxerr[Debug::INIT] << "Initializing key mappings..." << endl;

	if (prim_lang.empty() && sec_lang.empty())
		keymapon = false;
	else
		keymapon = on;

	KeyMapOn(keymapon);

	if (keymapon)
		KeyMapPrim();

	trans.SetPrimary(prim_lang);
	trans.SetSecondary(sec_lang);
	trans.setCharset(lyxrc.font_norm);
}
