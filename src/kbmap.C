/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <cstring>
#include "support/lstrings.h"
#include "gettext.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "kbmap.h"
#include "debug.h"

// The only modifiers that we handle. We want to throw away things
// like NumLock. 
enum { ModsMask = ShiftMask | ControlMask | Mod1Mask};


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
static
void printKeysym(KeySym key, unsigned int mod, string & buf)
{
	mod &= ModsMask;

	char * s = XKeysymToString(key);
	
	if (mod & ShiftMask) buf += "S-";
	if (mod & ControlMask) buf += "C-";
	if (mod & Mod1Mask) buf += "M-";
	if (s) buf += s;
}


/* ---F+------------------------------------------------------------------ *\
   Function  : printKeyTab
   Called by : kb_keymap::print
   Purpose   : print the keysyms found in the given key table. RVDK_PATCH_5
   Parameters: tabPt  - keytable pointer
               buf    - string where the result goes
               maxLen - length of string (including '\0')
   Returns   : length of printed string.
\* ---F------------------------------------------------------------------- */

static
void printKeyTab(kb_key * tabPt, string & buf)
{
	unsigned int ksym, mod;
	
	/* -------> Print each of the slots into buf. */
	for( ; (tabPt->code & 0xffff) != NoSymbol; ++tabPt) {
		ksym =  tabPt->code;
		mod  =  tabPt->mod & 0xffff;
		
		printKeysym(ksym, mod, buf);
		buf += ' ';
	}
}


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

int kb_sequence::addkey(KeySym key,
			unsigned int mod, unsigned int nmod /*= 0*/)
{
	if(length < 0) length = 0;

	if(length + 1 >= size) {
		unsigned int * nseq = new unsigned int[size + KB_PREALLOC];
		size += KB_PREALLOC;
		memcpy(nseq, sequence, length * sizeof(unsigned int));
		if(sequence != staticseq) delete sequence;
		sequence = nseq;
		nseq = new unsigned int[size];
		memcpy(nseq, modifiers, length * sizeof(unsigned int));
		if(modifiers != staticmod) delete modifiers;
		modifiers = nseq;
	}

	modifiers[length]  = mod + (nmod << 16);
	sequence[length++] = key;
   
	if(curmap)
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

int kb_sequence::parse(char const * s)
{
	if(!s[0]) return 1;

	int i = 0;
	unsigned int mod = 0, nmod = 0;
	while(s[i]) {
		if(s[i] && (s[i]) <= ' ') ++i;
		if(!s[i]) break;
		
		if(s[i + 1] == '-')	{ // is implicit that s[i] == true
			switch(s[i]) {
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
		} else if(s[i] == '~' && s[i + 1] && s[i + 2] == '-') {
			switch(s[i + 1]) {
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
			int j = i;
			for(; s[j] && s[j] > ' '; ++j)
				tbuf += s[j];    // (!!!check bounds :-)
			
			KeySym key = XStringToKeysym(tbuf.c_str());
			if(key == NoSymbol) {
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
	if ( l < 0 && !when_defined ) l = -l;
	
	for(int i = 0; i < l; ++i) {
		key = sequence[i];
		mod = modifiers[i] & 0xffff;

		printKeysym(key, mod, buf);  // RVDK_PATCH_5

		if(i + 1 < l) {  // append a blank
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

KeySym kb_sequence::getsym()
{
	int l = length;
	if(l == 0) return NoSymbol;
	if(l < 0) l = -l;
	return sequence[l - 1];
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_sequence::getiso
    Called by : [user]
    Purpose   : return iso character code of last key, if any
    Parameters: none
    Returns   : iso code or 0 if none
\* ---F------------------------------------------------------------------- */

char kb_sequence::getiso()
{
	int c = getsym();
	
	if(c > 0xff)
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


// === kb_keymap methods ================================================== 

// This binds a key to an action
int kb_keymap::bind(char const * seq, int action)
{
	kb_sequence k;

	int res = k.parse(seq);
	if (!res) {
		defkey(&k, action);
	} else
		lyxerr[Debug::KBMAP] << "Parse error at position " << res
				     << " in key sequence '" << seq << "'."
				     << endl;
	return res;
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_keymap::lookup
    Called by : [user], kb_sequence::add()
    Purpose   : look up a key press in a given keymap
    Parameters: key - the keysym of the key press
                mod - the modifier mask of the keypress
                seq - the key-sequence retrieved so far
    Returns   : user defined action; 0 for prefix key, -1 if key not found
\* ---F------------------------------------------------------------------- */

int kb_keymap::lookup(KeySym key, unsigned int mod, kb_sequence * seq)
{
	unsigned int ksym, msk1, msk0;
	kb_key * tab;

	//suppress modifier bits we do not handle
	mod &= ModsMask;

	if(!table) {
		// error - no keymap defined:
		seq->curmap = seq->stdmap;
		seq->delseq();
		return -1;
	}

	tab = table;

	// --- now search the list of keys ---

	for(; (tab->code & 0xffff) != NoSymbol; ++tab) {
		ksym =  tab->code;
		msk1 =  tab->mod      & 0xffff;
		msk0 = (tab->mod >> 16) & 0xffff;

		if(ksym == key && (mod & ~msk0) == msk1) {
			// match found:
			if(tab->table) {
				// this is a prefix key - set new map
				seq->curmap = tab->table;
				return 0;
			} else {
				// final key - reset map
				seq->curmap = seq->stdmap;
				seq->delseq();
				return tab->action; // ... and return action
			}
		}
	}
	
	// error - key not found:
	seq->curmap = seq->stdmap;
	seq->delseq();
	return -1;
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_keymap::print
    Called by : [user]
    Purpose   : Prints all the available keysyms. RVDK_PATCH_5
    Parameters: buf    - string where output goes.
               maxLen - available length in string, including `\0'.
    Returns   : updated maxLen.
\* ---F------------------------------------------------------------------- */

void kb_keymap::print(string & buf) const
{
	// Return when keymap has no table.
	if (!table) return;
   
	// Process each of its slots recursively and return.
	printKeyTab(table, buf);
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_keymap::defkey
    Called by : [user]
    Purpose   : define an action for a key sequence
    Parameters: seq    - the key sequence
                action - the action to be defined
                idx    - recursion depth
    Returns   : 0 if ok.
\* ---F------------------------------------------------------------------- */

int kb_keymap::defkey(kb_sequence * seq, int action, int idx /*= 0*/)
{
	unsigned int code = seq->sequence[idx];
	if(code == NoSymbol) return -1;

	unsigned int modmsk = seq->modifiers[idx];
	kb_key  * tab, ** ptab;
	// --- get list------------------------------------------------------
	if(!table) {
		// If we don't have any yet, make an empty one
		table = new kb_key[KB_PREALLOC];
		table[0].code = NoSymbol;
		tab   =  table;
		ptab  = &table;
		size  = KB_PREALLOC;
	} else {
		tab  =  table;
		ptab = &table;
	}

	// --- check if key is already there --------------------------------

	kb_key * t;
	int tsize;
	for(t = tab, tsize = 1; t->code != NoSymbol; ++t, ++tsize) {
		if(code == t->code && modmsk == t->mod) { // -- overwrite binding ---
			if(idx + 1 == seq->length) {
				string buf;
				seq->print(buf, true);
				lyxerr[Debug::KEY]
					<< "Warning: New binding for '"
					<< buf 
					<< "' is overriding old binding..."
					<< endl;

				if(t->table) {
					delete t->table;
					t->table = 0;
				}
				t->action = action;
				return 0;
			} else if (!t->table) {
				string buf;
				seq->print(buf, true);
				lyxerr << "Error: New binding for '" << buf
				       << "' is overriding old binding..."
				       << endl;
				return -1;
			} else
				return t->table->defkey(seq, action, idx + 1);
		}
	}

	// --- extend list if necessary -------------------------------------

	if(tsize % KB_PREALLOC == 0) {
		kb_key * nt = new kb_key[tsize + KB_PREALLOC];
		// Set to 0 as table is used uninitialised later (thornley)
		nt[tsize].table = 0;
		memcpy(nt, tab, tsize * sizeof(kb_key));
		*ptab = nt;
		delete[] tab;
		tab = nt;
		if(size >= 0) size = tsize + KB_PREALLOC;
	}

	// --- add action ---------------------------------------------------

	tab[tsize--].code = NoSymbol;
	tab[tsize].code = code;
	tab[tsize].mod  = modmsk;
	kb_key * newone = &tab[tsize];
	
	// --- define rest of sequence --------------------------------------

	if(idx+1 == seq->length) {
		newone->action = action;
		newone->table  = 0;
		return 0;
	} else {
		newone->table = new kb_keymap;
		int res = newone->table->defkey(seq, action, idx+1);
		return res;
	}
}


/* ---F+------------------------------------------------------------------ *\
    Function  : kb_keymap::~kb_keymap
    Called by : [destructor]
    Purpose   : free keymap and its descendents
    Parameters: none
    Returns   : nothing
\* ---F------------------------------------------------------------------- */

kb_keymap::~kb_keymap()
{
	if(!table) return;
	for(kb_key * t = table; t->code != NoSymbol; ++t)
		if(t->table)
			delete t->table;
	delete table;
}


string keyname(kb_key k)
{
	string buf;
	printKeysym(k.code, k.mod, buf);
	return buf;
}


// Finds a key for a keyaction, if possible
string kb_keymap::findbinding(int act) const
{
	string res;
	if (!table) return res;

	for(kb_key * t = table; t->code != NoSymbol; ++t) {
		if(t->table) {
			string suffix = t->table->findbinding(act);
			suffix = strip(suffix, ' ');
			suffix = strip(suffix, ']');
			suffix = frontStrip(suffix, '[');
			if (!suffix.empty()) {
				res += "[" + keyname(*t) + " " + suffix + "] ";
			}
		} else if (t->action == act) {
			res += "[" + keyname(*t) + "] ";
		}
	}
	return res;
}


/* === End of File: kbmap.C ============================================== */
