/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
//#include <cstring>
#include <X11/Xlib.h>

#include "gettext.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "kbsequence.h"
#include "kbmap.h"
#include "commandtags.h"
#include "debug.h"


using std::vector;
using std::endl;
using std::hex;
using std::dec;


// The only modifiers that we handle. We want to throw away things
// like NumLock.
enum { ModsMask = ShiftMask | ControlMask | Mod1Mask };


int kb_sequence::addkey(unsigned int key, unsigned int mod, unsigned int nmod)
{
	// adding a key to a deleted sequence
	// starts a new sequence
	if (deleted_) {
		deleted_ = false;
		length_ = 0;
		sequence.clear();
		modifiers.clear();
	}

	modifiers.push_back(mod + (nmod << 16));
	sequence.push_back(key);
	++length_;

	if (curmap) {
		return curmap->lookup(key, mod, this);
	}
	
	return LFUN_UNKNOWN_ACTION;
}


string::size_type kb_sequence::parse(string const & s)
{
	if (s.empty()) return 1;

	string::size_type i = 0;
	unsigned int mod = 0;
	unsigned int nmod = 0;
	while (i < s.length()) {
		if (s[i] == ' ')
			++i;
		if (i >= s.length())
			break;
		
		if (i + 1 < s.length() && s[i + 1] == '-') {
			switch (s[i]) {
			case 's': case 'S':
				mod |= ShiftMask;
				i += 2;
				continue;
			case 'c': case 'C':
				mod |= ControlMask;
				i += 2;
				continue;
			case 'm': case 'M':
				mod |= Mod1Mask;
				i += 2;
				continue;
			default:
				return i + 1;
			}
		} else if (i + 2 < s.length() && s[i] == '~'
			   && s[i + 2] == '-') {
			switch (s[i + 1]) {
			case 's': case 'S':
				nmod |= ShiftMask;
				i += 3;
				continue;
			case 'c': case 'C':
				nmod |= ControlMask;
				i += 3;
				continue;
			case 'm': case 'M':
				nmod |= Mod1Mask;
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
			
			KeySym key = XStringToKeysym(tbuf.c_str());
			if (key == NoSymbol) {
				lyxerr[Debug::KBMAP]
					<< "kbmap.C: No such keysym: "
					<< tbuf << endl;
				return j;
			}
			i = j;
			
			addkey(key, mod, nmod);
			mod = 0;
		}
	}
	
	// empty sequence?
	if (!length_)
		return 0;

	// everything is fine
	return string::npos;
}


string const kb_sequence::print() const
{
	string buf;

	//if (deleted_)
	//	return buf;
	
	for (vector<unsigned int>::size_type i = 0; i < length_; ++i) {
		buf += kb_keymap::printKeysym(sequence[i], modifiers[i] & 0xffff);

		// append a blank
		if (i + 1 < length_) {
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


unsigned int kb_sequence::getsym() const
{
	if (length_ == 0) return NoSymbol;
	return sequence[length_ - 1];
}


char kb_sequence::getiso() const
{
	unsigned int const c = getsym();

	lyxerr[Debug::KBMAP] << "Raw keysym: "
			     << hex << c << dec << endl;
	lyxerr[Debug::KBMAP] << "byte 3: "
			     << hex << (c & 0xff00) << dec
			     << endl;
	return kb_keymap::getiso(c);
}


void kb_sequence::reset()
{
	mark_deleted();
	curmap = stdmap;
}

void kb_sequence::clear()
{
	length_ = 0;
	reset();
}
