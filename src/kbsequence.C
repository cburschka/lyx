/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <cstring>
#include <X11/Xlib.h>

#include "gettext.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "kbsequence.h"
#include "kbmap.h"
#include "debug.h"

using std::endl;

// The only modifiers that we handle. We want to throw away things
// like NumLock. 
enum { ModsMask = ShiftMask | ControlMask | Mod1Mask };


// === static functions =================================================== 


/* ---F+------------------------------------------------------------------ *\
   Function  : printKeysym
   Called by : kb_sequence::print and printKeyMap. RVDK_PATCH_5
   Purpose   : prints a keysym, including modifiers.
   Parameters: key    - keysym
               mod    - modifiers
               buf    - string where the result goes
	       maxlen - length of string (including '\0')
   Returns   : length of printed string if ok, 0 otherwise.
\* ---F------------------------------------------------------------------- */
extern
void printKeysym(unsigned int key, unsigned int mod, string & buf);


// === kb_sequence methods ================================================ 

/* ---F+------------------------------------------------------------------ *\
    Function  : kb_sequence::addkey
    Called by : [user]
    Purpose   : add a key to the sequence, look up in map and return action
    Parameters: key  - keysym of key
                mod  - modifier mask
                nmod - modifier veto mask (unused now)
    Returns   : action or -1 if error (no map defined or key not found)
\* ---F------------------------------------------------------------------- */

int kb_sequence::addkey(unsigned int key,
			unsigned int mod, unsigned int nmod /*= 0*/)
{
	if (length < 0) {
		length = 0;
		sequence.clear();
		modifiers.clear();
	}

	modifiers.push_back(mod + (nmod << 16));
	sequence.push_back(key);
	++length;

	if (curmap)
		return curmap->lookup(key, mod, this);
	
	return -1;
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_sequence::parse
    Called by : [user]
    Purpose   : parse a string that holds a key sequence and add the keys
    Parameters: s - string holding the key sequence
    Returns   : 0 - if ok, error pos if error
    Note      : Keys must be separated with whitespace;
                Use the keysym names used by XStringToKeysym
                Prefixes are S-, C-, M- for shift, control, meta
\* ---F------------------------------------------------------------------- */

int kb_sequence::parse(string const & s)
{
	if (s.empty()) return 1;

	string::size_type i = 0;
	unsigned int mod = 0, nmod = 0;
	while (i < s.length()) {
		if (s[i] && (s[i]) <= ' ') ++i;
		if (i >= s.length()) break;
		
		if (s[i + 1] == '-')	{ // is implicit that s[i] == true
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
		} else if (s[i] == '~' && s[i + 1] && s[i + 2] == '-') {
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
			for (; j < s.length() && s[j] > ' '; ++j)
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
			nmod = 0;
		}
	}
	return 0;
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_sequence::print
    Called by : [user]
    Purpose   : print the currently defined sequence into a string
    Parameters: buf           - string where the result goes
                maxlen        - length of string (including '\0')
                when_defined  - only  print when sequence is real: length > 0.
    Returns   : 0, if ok, -1 if string too long
\* ---F------------------------------------------------------------------- */

int kb_sequence::print(string & buf, bool when_defined) const
{
	KeySym key;
	unsigned int mod;
	int l = length;
	if (l < 0 && !when_defined ) l = -l;
	
	for (int i = 0; i < l; ++i) {
		key = sequence[i];
		mod = modifiers[i] & 0xffff;

		printKeysym(key, mod, buf);  // RVDK_PATCH_5

		if (i + 1 < l) {  // append a blank
			buf += ' ';
		}
	}
	return 0;
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_sequence::printOptions
    Called by : [user]
    Purpose   : print the available key options from the current state in the
                sequence. RVDK_PATCH_5
    Parameters: buf    - string where the result goes
                maxlen - length of string (including '\0')
    Returns   : 0, if ok, -1 if string too long
\* ---F------------------------------------------------------------------- */

int kb_sequence::printOptions(string & buf) const
{
	print(buf, true);
	
	if (!curmap) return -1;
	buf += _("   options: ");
	curmap->print(buf);
	return 0;
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_sequence::delseq
    Called by : [user]
    Purpose   : mark the sequence as deleted
    Parameters: none
    Returns   : nothing
\* ---F------------------------------------------------------------------- */

void kb_sequence::delseq()
{
	// negative length marks sequence as deleted, but we can still
	// print() it or retrieve the last char using getiso()
	length = -length;
}


/* ---F+------------------------------------------------------------------ *\
   Function  : kb_sequence::getsym
   Called by : [user], getiso
   Purpose   : get the keysym of the last key in sequence
   Parameters: none
   Returns   : keysym
\* ---F------------------------------------------------------------------- */

unsigned int kb_sequence::getsym() const
{
	int l = length;
	if (l == 0) return NoSymbol;
	if (l < 0) l = -l;
	return sequence[l - 1];
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_sequence::getiso
    Called by : [user]
    Purpose   : return iso character code of last key, if any
    Parameters: none
    Returns   : iso code or 0 if none
\* ---F------------------------------------------------------------------- */

char kb_sequence::getiso() const
{
	int c = getsym();
	
	if (c > 0xff)
		return '\0';
	return c;
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_sequence::reset
    Called by : [user]
    Purpose   : reset sequence to initial state. RVDK_PATCH_5
    Parameters: none
    Returns   : void
\* ---F------------------------------------------------------------------- */

void kb_sequence::reset()
{
	delseq();
	curmap = stdmap;
	if (length > 0) length = -length;
}

/* === End of File: kbmap.C ============================================== */
