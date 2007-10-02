/**
 * \file KeyMap.cpp
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

#include "KeyMap.h"

#include "debug.h"
#include "KeySequence.h"
#include "LyXAction.h"
#include "Lexer.h"

#include "support/filetools.h"

#include <sstream>

using std::endl;
using std::string;


namespace lyx {

using support::FileName;
using support::i18nLibFileSearch;


string const KeyMap::printKeySym(KeySymbol const & key, KeyModifier mod)
{
	string buf;

	string const s = key.getSymbolName();

	if (mod & ShiftModifier)
		buf += "S-";
	if (mod & ControlModifier)
		buf += "C-";
	if (mod & AltModifier)
		buf += "M-";

	buf += s;
	return buf;
}


size_t KeyMap::bind(string const & seq, FuncRequest const & func)
{
	LYXERR(Debug::KBMAP) << "BIND: Sequence `"
	       << seq << "' Action `"
	       << func.action << '\'' << endl;

	KeySequence k(0, 0);

	string::size_type const res = k.parse(seq);
	if (res == string::npos) {
		defkey(&k, func);
	} else {
		LYXERR(Debug::KBMAP) << "Parse error at position " << res
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


bool KeyMap::read(string const & bind_file)
{
	const int bindCount = sizeof(bindTags) / sizeof(keyword_item);

	Lexer lexrc(bindTags, bindCount);
	if (lyxerr.debugging(Debug::PARSER))
		lexrc.printTable(lyxerr);

	FileName const tmp(i18nLibFileSearch("bind", bind_file, "bind"));
	lexrc.setFile(tmp);
	if (!lexrc.isOK()) {
		lyxerr << "KeyMap::read: cannot open bind file:"
		       << tmp << endl;
		return false;
	}

	LYXERR(Debug::KBMAP) << "Reading bind file:" << tmp << endl;

	bool error = false;
	while (lexrc.isOK()) {
		switch (lexrc.lex()) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown tag `$$Token'");
			error = true;
			continue;
		case Lexer::LEX_FEOF:
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

			FuncRequest func = lyxaction.lookupFunc(cmd);
			if (func. action == LFUN_UNKNOWN_ACTION) {
				lexrc.printError("BN_BIND: Unknown LyX"
						 " function `$$Token'");
				error = true;
				break;
			}

			bind(seq, func);
			break;
		}
		case BN_BINDFILE:
			if (lexrc.next()) {
				string const tmp(lexrc.getString());
				error |= !read(tmp);
			} else {
				lexrc.printError("BN_BINDFILE: Missing file name");
				error = true;
				break;

			}
			break;
		}
	}

	if (error)
		lyxerr << "KeyMap::read: error while reading bind file:"
		       << tmp << endl;
	return !error;
}


FuncRequest const & KeyMap::lookup(KeySymbol const &key,
		  KeyModifier mod, KeySequence * seq) const
{
	static FuncRequest const unknown(LFUN_UNKNOWN_ACTION);

	if (table.empty()) {
		seq->curmap = seq->stdmap;
		seq->mark_deleted();
		return unknown;
	}

	Table::const_iterator end = table.end();
	for (Table::const_iterator cit = table.begin(); cit != end; ++cit) {
		KeyModifier mask = cit->mod.second;
		KeyModifier check = static_cast<KeyModifier>(mod & ~mask);

		if (cit->code == key && cit->mod.first == check) {
			// match found
			if (cit->table.get()) {
				// this is a prefix key - set new map
				seq->curmap = cit->table.get();
				static FuncRequest prefix(LFUN_COMMAND_PREFIX);
				return prefix;
			} else {
				// final key - reset map
				seq->curmap = seq->stdmap;
				seq->mark_deleted();
				return cit->func;
			}
		}
	}

	// error - key not found:
	seq->curmap = seq->stdmap;
	seq->mark_deleted();

	return unknown;
}


docstring const KeyMap::print(bool forgui) const
{
	docstring buf;
	Table::const_iterator end = table.end();
	for (Table::const_iterator cit = table.begin(); cit != end; ++cit) {
		buf += cit->code.print(cit->mod.first, forgui);
		buf += ' ';
	}
	return buf;
}


void KeyMap::defkey(KeySequence * seq, FuncRequest const & func, unsigned int r)
{
	KeySymbol code = seq->sequence[r];
	if (!code.isOK())
		return;

	KeyModifier const mod1 = seq->modifiers[r].first;
	KeyModifier const mod2 = seq->modifiers[r].second;

	// check if key is already there
	Table::iterator end = table.end();
	for (Table::iterator it = table.begin(); it != end; ++it) {
		if (code == it->code
		    && mod1 == it->mod.first
		    && mod2 == it->mod.second) {
			// overwrite binding
			if (r + 1 == seq->length()) {
				LYXERR(Debug::KBMAP)
					<< "Warning: New binding for '"
					<< to_utf8(seq->print(false))
					<< "' is overriding old binding..."
					<< endl;
				if (it->table.get()) {
					it->table.reset();
				}
				it->func = func;
				it->func.origin = FuncRequest::KEYBOARD;
				return;
			} else if (!it->table.get()) {
				lyxerr << "Error: New binding for '"
				       << to_utf8(seq->print(false))
				       << "' is overriding old binding..."
					       << endl;
				return;
			} else {
				it->table->defkey(seq, func, r + 1);
				return;
			}
		}
	}

	Table::iterator newone = table.insert(table.end(), Key());
	newone->code = code;
	newone->mod = seq->modifiers[r];
	if (r + 1 == seq->length()) {
		newone->func = func;
		newone->func.origin = FuncRequest::KEYBOARD;
		newone->table.reset();
	} else {
		newone->table.reset(new KeyMap);
		newone->table->defkey(seq, func, r + 1);
	}
}


docstring const KeyMap::printbindings(FuncRequest const & func) const
{
	odocstringstream res;
	Bindings bindings = findbindings(func);
	for (Bindings::const_iterator cit = bindings.begin();
	     cit != bindings.end() ; ++cit)
		res << '[' << cit->print(true) << ']';
	return res.str();
}


KeyMap::Bindings KeyMap::findbindings(FuncRequest const & func) const
{
	return findbindings(func, KeySequence(0, 0));
}


KeyMap::Bindings KeyMap::findbindings(FuncRequest const & func,
			KeySequence const & prefix) const
{
	Bindings res;
	if (table.empty()) return res;

	Table::const_iterator end = table.end();
	for (Table::const_iterator cit = table.begin();
	    cit != end; ++cit) {
		if (cit->table.get()) {
			KeySequence seq = prefix;
			seq.addkey(cit->code, cit->mod.first);
			Bindings res2 =
				cit->table->findbindings(func, seq);
			res.insert(res.end(), res2.begin(), res2.end());
		} else if (cit->func == func) {
			KeySequence seq = prefix;
			seq.addkey(cit->code, cit->mod.first);
			res.push_back(seq);
		}
	}

	return res;
}


} // namespace lyx
