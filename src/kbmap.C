/**
 * \file kbmap.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "kbmap.h"
#include "commandtags.h"
#include "kbsequence.h"
#include "debug.h"

#include <X11/Xlib.h>

using std::endl;

// The only modifiers that we handle. We want to throw away things
// like NumLock.
enum { ModsMask = ShiftMask | ControlMask | Mod1Mask };


string const kb_keymap::printKeysym(unsigned int key, key_modifier::state mod)
{
	string buf;

	char const * const s = XKeysymToString(key);

	if (mod & key_modifier::shift) buf += "S-";
	if (mod & key_modifier::ctrl) buf += "C-";
	if (mod & key_modifier::alt) buf += "M-";
	if (s) buf += s;
	return buf;
}


char kb_keymap::getiso(unsigned int c)
{
	switch (c & 0x0000FF00) {
		// latin 1 byte 3 = 0
	case 0x00000000: break;
		// latin 2 byte 3 = 1
	case 0x00000100:
		// latin 3 byte 3 = 2
	case 0x00000200:
		// latin 4 byte 3 = 3
	case 0x00000300:
		// cyrillic KOI8 & Co
	case 0x00000600:
		// greek
	case 0x00000700:
		// latin 8 byte 3 = 18 (0x12)
	case 0x00001200:
		// latin 9 byte 3 = 19 (0x13)
	case 0x00001300:
		c &= 0x000000FF;
		break;
	default:
		c = 0;
	}
	return c;
}

string const kb_keymap::printKey(kb_key const & key) const
{
	return printKeysym(key.code, key.mod.first);
}


string::size_type kb_keymap::bind(string const & seq, int action)
{
	if (lyxerr.debugging(Debug::KBMAP)) {
		lyxerr << "BIND: Sequence `"
		       << seq << "' Action `"
		       << action << "'" << endl;
	}

	kb_sequence k(0, 0);

	string::size_type const res = k.parse(seq);
	if (res == string::npos) {
		defkey(&k, action);
	} else {
		lyxerr[Debug::KBMAP] << "Parse error at position " << res
				     << " in key sequence '" << seq << "'."
				     << endl;
	}

	return res;
}


int kb_keymap::lookup(unsigned int key,
		      key_modifier::state mod, kb_sequence * seq) const
{
	if (table.empty()) {
		seq->curmap = seq->stdmap;
		seq->mark_deleted();
		return LFUN_UNKNOWN_ACTION;
	}

	for (Table::const_iterator cit = table.begin();
	     cit != table.end(); ++cit) {
		key_modifier::state mask(cit->mod.second);
		key_modifier::state check =
			static_cast<key_modifier::state>(mod & ~mask);
 
		if (cit->code == key && cit->mod.first == check) {
			// match found
			if (cit->table.get()) {
				// this is a prefix key - set new map
				seq->curmap = cit->table.get();
				return LFUN_PREFIX;
			} else {
				// final key - reset map
				seq->curmap = seq->stdmap;
				seq->mark_deleted();
				return cit->action;
			}
		}
	}

	// error - key not found:
	seq->curmap = seq->stdmap;
	seq->mark_deleted();
	return LFUN_UNKNOWN_ACTION;
}


string const kb_keymap::print() const
{
	string buf;
	for (Table::const_iterator cit = table.begin();
	     cit != table.end(); ++cit) {
		buf += printKey((*cit));
		buf += ' ';
	}
	return buf;
}


void kb_keymap::defkey(kb_sequence * seq, int action, unsigned int r)
{
	unsigned int const code = seq->sequence[r];
	if (code == NoSymbol) return;

	key_modifier::state const mod1 = seq->modifiers[r].first;
	key_modifier::state const mod2 = seq->modifiers[r].second;

	// check if key is already there
	for (Table::iterator it = table.begin(); it != table.end(); ++it) {
		if (code == it->code && mod1 == it->mod.first && mod2 == it->mod.second) {
			// overwrite binding
			if (r + 1 == seq->length()) {
				lyxerr[Debug::KBMAP]
					<< "Warning: New binding for '"
					<< seq->print()
					<< "' is overriding old binding..."
					<< endl;
				if (it->table.get()) {
					it->table.reset();
				}
				it->action = action;
				return;
			} else if (!it->table.get()) {
				lyxerr << "Error: New binding for '" << seq->print()
				       << "' is overriding old binding..."
					       << endl;
				return;
			} else {
				it->table->defkey(seq, action, r + 1);
				return;
			}
		}
	}

	Table::iterator newone = table.insert(table.end(), kb_key());
	newone->code = code;
	newone->mod = seq->modifiers[r];
	if (r + 1 == seq->length()) {
		newone->action = action;
		newone->table.reset();
		return;
	} else {
		newone->table.reset(new kb_keymap);
		newone->table->defkey(seq, action, r + 1);
		return;
	}
}


string const kb_keymap::findbinding(int act, string const & prefix) const
{
	string res;
	if (table.empty()) return res;

	Table::const_iterator end = table.end();
	for (Table::const_iterator cit = table.begin();
	    cit != end; ++cit) {
		if (cit->table.get()) {
			res += cit->table->findbinding(act,
						       prefix
						       + printKey((*cit))
						       + " ");
		} else if (cit->action == act) {
			res += "[";
			res += prefix + printKey((*cit));
			res += "] ";
		}
	}
	return res;
}
