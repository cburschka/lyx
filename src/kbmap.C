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

#ifdef __GNUG__
#pragma implementation
#endif

#include "kbmap.h"
#include "kbsequence.h"
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
void printKeysym(unsigned int key, unsigned int mod, string & buf)
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

void kb_keymap::printKey(kb_key const & key, string & buf)
{
	printKeysym(key.code, key.mod & 0xffff, buf);
}


// This binds a key to an action
int kb_keymap::bind(string const & seq, int action)
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

int kb_keymap::lookup(unsigned int key,
		      unsigned int mod, kb_sequence * seq) const
{
	if (table.empty()) {
		seq->curmap = seq->stdmap;
		seq->delseq();
		return -1;
	}

	unsigned int msk1, msk0;
	//suppress modifier bits we do not handle
	mod &= ModsMask;

	for (Table::const_iterator cit = table.begin();
	     cit != table.end(); ++cit) {
		msk1 = (*cit).mod & 0xffff;
		msk0 = ((*cit).mod >> 16) & 0xffff;
		if ((*cit).code == key && (mod & ~msk0) == msk1) {
			// math found:
			if ((*cit).table) {
				// this is a prefix key - set new map
				seq->curmap = (*cit).table;
				return 0;
			} else {
				// final key - reset map
				seq->curmap = seq->stdmap;
				seq->delseq();
				return (*cit).action;
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
	for (Table::const_iterator cit = table.begin();
	     cit != table.end(); ++cit) {
		printKey((*cit), buf);
		buf += ' ';
	}
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

	// --- check if key is already there --------------------------------
	if (table.size() != 0) { // without this I get strange crashes
		Table::iterator end = table.end();
	for (Table::iterator it = table.begin(); it != end; ++it) {
		if (code == (*it).code && modmsk == (*it).mod) {
			// overwrite binding
			if (idx + 1 == seq->length) {
				string buf;
				seq->print(buf, true);
				lyxerr[Debug::KBMAP]
					<< "Warning: New binding for '"
					<< buf
					<< "' is overriding old binding..."
					<< endl;
				if((*it).table) {
					delete (*it).table;
					(*it).table = 0;
				}
				(*it).action = action;
				return 0;
			} else if (!(*it).table) {
				string buf;
				seq->print(buf, true);
				lyxerr << "Error: New binding for '" << buf
				       << "' is overriding old binding..."
				       << endl;
				return -1;
			} else {
				return (*it).table->defkey(seq, action,
							   idx + 1);
			}
		}
	}
	}
	
	Table::iterator newone = table.insert(table.end(), kb_key());
	(*newone).code = code;
	(*newone).mod = modmsk;
	if (idx + 1 == seq->length) {
		(*newone).action = action;
		(*newone).table = 0;
		return 0;
	} else {
		(*newone).table = new kb_keymap;
		return (*newone).table->defkey(seq, action, idx + 1);
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
	// This could be done by a destructor in kb_key.
	Table::iterator end = table.end();
	for (Table::iterator it = table.begin(); it != end; ++it) {
		delete (*it).table;
	}
}


string const kb_keymap::keyname(kb_key const & k)
{
	string buf;
	printKeysym(k.code, k.mod, buf);
	return buf;
}


// Finds a key for a keyaction, if possible
string const kb_keymap::findbinding(int act) const
{
	string res;
	if (table.empty()) return res;

	Table::const_iterator end = table.end();
	for (Table::const_iterator cit = table.begin();
	    cit != end; ++cit) {
		if ((*cit).table) {
			string suffix = (*cit).table->findbinding(act);
			suffix = strip(suffix, ' ');
			suffix = strip(suffix, ']');
			suffix = frontStrip(suffix, '[');
			if (!suffix.empty()) {
				res += "[" + keyname((*cit)) + " "
					+ suffix + "] ";
			}
		} else if ((*cit).action == act) {
			res += "[";
			res += keyname((*cit));
			res += "] ";
		}
	}
	return res;
}

/* === End of File: kbmap.C ============================================== */
