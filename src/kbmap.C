/**
 * \file kbmap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "kbmap.h"
#include "kbsequence.h"
#include "LyXAction.h"
#include "support/filetools.h"
#include "lyxlex.h"
#include "debug.h"

using std::endl;
using lyx::support::i18nLibFileSearch;

string const kb_keymap::printKeysym(LyXKeySymPtr key,
				    key_modifier::state mod)
{
	string buf;

	string const s = key->getSymbolName();

	if (mod & key_modifier::shift)
		buf += "S-";
	if (mod & key_modifier::ctrl)
		buf += "C-";
	if (mod & key_modifier::alt)
		buf += "M-";

	buf += s;
	return buf;
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
		       << action << '\'' << endl;
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


namespace {

enum BindTags {
	BN_BIND,
	BN_BINDFILE
};

keyword_item bindTags[] = {
	{ "\\bind", BN_BIND },
	{ "\\bind_file", BN_BINDFILE }
};

}


bool kb_keymap::read(string const & bind_file) 
{
	const int bindCount = sizeof(bindTags) / sizeof(keyword_item);

	LyXLex lexrc(bindTags, bindCount);
	if (lyxerr.debugging(Debug::PARSER))
		lexrc.printTable(lyxerr);

	string const tmp = i18nLibFileSearch("bind", bind_file, "bind");
	lexrc.setFile(tmp);
	if (!lexrc.isOK()) {
		lyxerr << "kb_keymap::read: cannot open bind file:" 
		       << tmp << endl;
		return false;
	}

	lyxerr[Debug::KBMAP] << "Reading bind file:" << tmp << endl;

	bool error = false;
	while (!error && lexrc.isOK()) {
		switch (lexrc.lex()) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown tag `$$Token'");
			error = true;
			continue;
		case LyXLex::LEX_FEOF:
			continue;
		case BN_BIND:
		{
			string seq, cmd;
			
			if (lexrc.next()) {
				seq = lexrc.getString();
			} else {
				lexrc.printError("BN_BIND: Missing key sequence");
				error = true;
				break;
			}
			
			if (lexrc.next(true)) {
				cmd = lexrc.getString();
			} else {
				lexrc.printError("BN_BIND: missing command");
				error = true;
				break;
			}
			
			int action = lyxaction.LookupFunc(cmd);
			if (!action == LFUN_UNKNOWN_ACTION) {
				lexrc.printError("BN_BIND: Unknown LyX"
						 " function `$$Token'");
				error = true;
				break;
			}
			
			bind(seq, kb_action(action));
			break;
		}
		case BN_BINDFILE:
			if (lexrc.next()) {
				string const tmp(lexrc.getString());
				error = !read(tmp);
			} else {
				lexrc.printError("BN_BINDFILE: Missing file name");
				error = true;
				break;

			}
			break;
		}
	}

	if (error)
		lyxerr << "kb_keymap::read: error while reading bind file:" 
		       << tmp << endl;
	return !error;
}


int kb_keymap::lookup(LyXKeySymPtr key,
		      key_modifier::state mod, kb_sequence * seq) const
{
	if (table.empty()) {
		seq->curmap = seq->stdmap;
		seq->mark_deleted();
		return LFUN_UNKNOWN_ACTION;
	}

	Table::const_iterator end = table.end();
	for (Table::const_iterator cit = table.begin(); cit != end; ++cit) {
		key_modifier::state mask(cit->mod.second);
		key_modifier::state check =
			static_cast<key_modifier::state>(mod & ~mask);

		if (*(cit->code) == *key && cit->mod.first == check) {
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
	Table::const_iterator end = table.end();
	for (Table::const_iterator cit = table.begin(); cit != end; ++cit) {
		buf += printKey((*cit));
		buf += ' ';
	}
	return buf;
}


void kb_keymap::defkey(kb_sequence * seq, int action, unsigned int r)
{
	LyXKeySymPtr code = seq->sequence[r];
	if (!code->isOK())
		return;

	key_modifier::state const mod1 = seq->modifiers[r].first;
	key_modifier::state const mod2 = seq->modifiers[r].second;

	// check if key is already there
	Table::iterator end = table.end();
	for (Table::iterator it = table.begin(); it != end; ++it) {
		if (*(code) == *(it->code)
		    && mod1 == it->mod.first
		    && mod2 == it->mod.second) {
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
						       + ' ');
		} else if (cit->action == act) {
			res += '[';
			res += prefix + printKey((*cit));
			res += "] ";
		}
	}
	return res;
}
