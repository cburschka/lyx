/**
 * \file kbsequence.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "gettext.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "frontends/mouse_state.h"
#include "frontends/LyXKeySymFactory.h"
#include "kbsequence.h"
#include "kbmap.h"
#include "commandtags.h"
#include "debug.h"

using std::make_pair;


int kb_sequence::addkey(LyXKeySymPtr key,
			key_modifier::state mod, key_modifier::state nmod)
{
	// adding a key to a deleted sequence
	// starts a new sequence
	if (deleted_) {
		deleted_ = false;
		sequence.clear();
		modifiers.clear();
	}

	modifiers.push_back(make_pair(mod, nmod));
	sequence.push_back(key);

	if (curmap) {
		return curmap->lookup(key, mod, this);
	}

	return LFUN_UNKNOWN_ACTION;
}


string::size_type kb_sequence::parse(string const & s)
{
	if (s.empty()) return 1;

	string::size_type i = 0;
	key_modifier::state mod = key_modifier::none;
	key_modifier::state nmod = key_modifier::none;
 
	while (i < s.length()) {
		if (s[i] == ' ')
			++i;
		if (i >= s.length())
			break;

		if (i + 1 < s.length() && s[i + 1] == '-') {
			switch (s[i]) {
			case 's': case 'S':
				mod |= key_modifier::shift;
				i += 2;
				continue;
			case 'c': case 'C':
				mod |= key_modifier::ctrl;
				i += 2;
				continue;
			case 'm': case 'M':
				mod |= key_modifier::alt;
				i += 2;
				continue;
			default:
				return i + 1;
			}
		} else if (i + 2 < s.length() && s[i] == '~'
			   && s[i + 2] == '-') {
			switch (s[i + 1]) {
			case 's': case 'S':
				nmod |= key_modifier::shift;
				i += 3;
				continue;
			case 'c': case 'C':
				nmod |= key_modifier::ctrl;
				i += 3;
				continue;
			case 'm': case 'M':
				nmod |= key_modifier::alt;
				i += 3;
				continue;
			default:
				return i + 2;
			}
		} else {
			string tbuf;
			string::size_type j = i;
			for (; j < s.length() && s[j] != ' '; ++j)
				tbuf += s[j];    // (!!!check bounds :-)

			LyXKeySymPtr key(LyXKeySymFactory::create());
			key->init(tbuf);
			
			if ( ! key->isOK() ) {
				return j;
			}

			i = j;

			addkey(key, mod, nmod);
			mod = key_modifier::none;
		}
	}

	// empty sequence?
	if (sequence.size() == 0)
		return 0;

	// everything is fine
	return string::npos;
}


string const kb_sequence::print() const
{
	string buf;

	//if (deleted_)
	//	return buf;

	KeySequence::size_type i, length = sequence.size();
	
	for (i = 0; i < length; ++i) {
		buf += kb_keymap::printKeysym(sequence[i], modifiers[i].first);

		// append a blank
		if (i + 1 < length) {
			buf += ' ';
		}
	}
	return buf;
}


string const kb_sequence::printOptions() const
{
	string buf;

	buf += print();

	if (!curmap)
		return buf;

	buf += _("   options: ");
	buf += curmap->print();
	return buf;
}


void kb_sequence::mark_deleted()
{
	deleted_ = true;
}


LyXKeySymPtr kb_sequence::getsym() const
{
	if (sequence.size() == 0)
		return LyXKeySymPtr(LyXKeySymFactory::create());
	return sequence.back();
}


char kb_sequence::getLastKeyEncoded() const
{
	return getsym()->getISOEncoded();
}


void kb_sequence::reset()
{
	mark_deleted();
	curmap = stdmap;
}

void kb_sequence::clear()
{
	sequence.clear();
	reset();
}
