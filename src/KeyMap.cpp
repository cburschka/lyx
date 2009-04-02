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

#include "KeySequence.h"
#include "LyXAction.h"
#include "Lexer.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/filetools.h"

#include <fstream>
#include <sstream>
#include <utility>

using namespace std;
using namespace lyx::support;

namespace lyx {


string const KeyMap::printKeySym(KeySymbol const & key, KeyModifier mod)
{
	string buf;

	string const s = key.getSymbolName();

	if (mod & ControlModifier)
		buf += "C-";
	if (mod & AltModifier)
		buf += "M-";
	if (mod & ShiftModifier)
		buf += "S-";

	buf += s;
	return buf;
}


size_t KeyMap::bind(string const & seq, FuncRequest const & func)
{
	LYXERR(Debug::KBMAP, "BIND: Sequence `" << seq << "' Action `"
	       << func.action << '\'');

	KeySequence k(0, 0);

	string::size_type const res = k.parse(seq);
	if (res == string::npos) {
		bind(&k, func);
	} else {
		LYXERR(Debug::KBMAP, "Parse error at position " << res
				     << " in key sequence '" << seq << "'.");
	}

	return res == string::npos ? 0 : res;
}


size_t KeyMap::unbind(string const & seq, FuncRequest const & func)
{
	KeySequence k(0, 0);

	string::size_type const res = k.parse(seq);
	if (res == string::npos)
		unbind(&k, func);
	else
		LYXERR(Debug::KBMAP, "Parse error at position " << res
				     << " in key sequence '" << seq << "'.");
	return res == string::npos ? 0 : res;
}


void KeyMap::bind(KeySequence * seq, FuncRequest const & func, unsigned int r)
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
				LYXERR(Debug::KBMAP, "Warning: New binding for '"
					<< to_utf8(seq->print(KeySequence::Portable))
					<< "' is overriding old binding...");
				if (it->table.get()) {
					it->table.reset();
				}
				it->func = func;
				it->func.origin = FuncRequest::KEYBOARD;
				return;
			} else if (!it->table.get()) {
				lyxerr << "Error: New binding for '"
				       << to_utf8(seq->print(KeySequence::Portable))
				       << "' is overriding old binding..."
				       << endl;
				return;
			} else {
				it->table->bind(seq, func, r + 1);
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
		newone->table->bind(seq, func, r + 1);
	}
}


void KeyMap::unbind(KeySequence * seq, FuncRequest const & func, unsigned int r)
{
	KeySymbol code = seq->sequence[r];
	if (!code.isOK())
		return;

	KeyModifier const mod1 = seq->modifiers[r].first;
	KeyModifier const mod2 = seq->modifiers[r].second;

	// check if key is already there
	Table::iterator end = table.end();
	Table::iterator remove = end;
	for (Table::iterator it = table.begin(); it != end; ++it) {
		if (code == it->code
		    && mod1 == it->mod.first
		    && mod2 == it->mod.second) {
			// remove
			if (r + 1 == seq->length()) {
				if (it->func == func) {
					remove = it;
					if (it->table.get())
						it->table.reset();
				}
			} else if (it->table.get()) {
				it->table->unbind(seq, func, r + 1);
				if (it->table->empty())
					remove = it;
				return;
			}
		}
	}
	if (remove != end)
		table.erase(remove);
}


FuncRequest KeyMap::getBinding(KeySequence const & seq, unsigned int r)
{
	KeySymbol code = seq.sequence[r];
	if (!code.isOK())
		return FuncRequest::unknown;

	KeyModifier const mod1 = seq.modifiers[r].first;
	KeyModifier const mod2 = seq.modifiers[r].second;

	// check if key is already there
	Table::iterator end = table.end();
	for (Table::iterator it = table.begin(); it != end; ++it) {
		if (code == it->code
		    && mod1 == it->mod.first
		    && mod2 == it->mod.second) {
			if (r + 1 == seq.length())
				return it->func;
			else if (it->table.get())
				return it->table->getBinding(seq, r + 1);
		}
	}
	return FuncRequest::unknown;
}


void KeyMap::clear()
{
	table.clear();
}


bool KeyMap::read(string const & bind_file, KeyMap * unbind_map)
{
	enum {
		BN_BIND,
		BN_BINDFILE,
		BN_UNBIND,
	};

	LexerKeyword bindTags[] = {
		{ "\\bind",      BN_BIND },
		{ "\\bind_file", BN_BINDFILE },
		{ "\\unbind",    BN_UNBIND },
	};

	Lexer lexrc(bindTags);
	if (lyxerr.debugging(Debug::PARSER))
		lexrc.printTable(lyxerr);

	FileName const tmp = i18nLibFileSearch("bind", bind_file, "bind");
	lexrc.setFile(tmp);
	if (!lexrc.isOK()) {
		LYXERR0("KeyMap::read: cannot open bind file:" << tmp);
		return false;
	}

	LYXERR(Debug::KBMAP, "Reading bind file:" << tmp);

	bool error = false;
	while (lexrc.isOK()) {
		switch (lexrc.lex()) {

		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown tag `$$Token'");
			error = true;
			continue;

		case Lexer::LEX_FEOF:
			continue;

		case BN_BIND: {
			if (!lexrc.next()) {
				lexrc.printError("BN_BIND: Missing key sequence");
				error = true;
				break;
			}
			string seq = lexrc.getString();

			if (!lexrc.next(true)) {
				lexrc.printError("BN_BIND: missing command");
				error = true;
				break;
			}
			string cmd = lexrc.getString();

			FuncRequest func = lyxaction.lookupFunc(cmd);
			if (func.action == LFUN_UNKNOWN_ACTION) {
				lexrc.printError("BN_BIND: Unknown LyX function `$$Token'");
				error = true;
				break;
			}

			bind(seq, func);
			break;
		}

		case BN_UNBIND: {
			if (!lexrc.next()) {
				lexrc.printError("BN_UNBIND: Missing key sequence");
				error = true;
				break;
			}
			string seq = lexrc.getString();

			if (!lexrc.next(true)) {
				lexrc.printError("BN_UNBIND: missing command");
				error = true;
				break;
			}
			string cmd = lexrc.getString();

			FuncRequest func = lyxaction.lookupFunc(cmd);
			if (func.action == LFUN_UNKNOWN_ACTION) {
				lexrc.printError("BN_UNBIND: Unknown LyX"
						 " function `$$Token'");
				error = true;
				break;
			}
			
			if (unbind_map)
				unbind_map->bind(seq, func);
			else
				unbind(seq, func);
			break;
		}

		case BN_BINDFILE:
			if (!lexrc.next()) {
				lexrc.printError("BN_BINDFILE: Missing file name");
				error = true;
				break;
			}
			string const tmp = lexrc.getString();
			error |= !read(tmp, unbind_map);
			break;
		}
	}

	if (error)
		LYXERR0("KeyMap::read: error while reading bind file:" << tmp);
	return !error;
}


void KeyMap::write(string const & bind_file, bool append, bool unbind) const
{
	ofstream os(bind_file.c_str(), 
		append ? (ios::app | ios::out) : ios::out);

	if (!append)
		os << "## This file is automatically generated by lyx\n"
		   << "## All modifications will be lost\n\n";
	
	string tag = unbind ? "\\unbind" : "\\bind";
	BindingList const list = listBindings(false);
	BindingList::const_iterator it = list.begin();
	BindingList::const_iterator it_end = list.end();
	for (; it != it_end; ++it) {
		FuncCode action = it->request.action;
		string arg = to_utf8(it->request.argument());

		os << tag << " \""
				<< to_utf8(it->sequence.print(KeySequence::BindFile))
				<< "\" \""
				<< lyxaction.getActionName(action)
				<< (arg.empty() ? "" : " ") << arg
				<< "\"\n";
	}
	os << "\n";
	os.close();
}


FuncRequest const & KeyMap::lookup(KeySymbol const &key,
		  KeyModifier mod, KeySequence * seq) const
{
	if (table.empty()) {
		seq->reset();
		return FuncRequest::unknown;
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
				seq->reset();
				return cit->func;
			}
		}
	}

	// error - key not found:
	seq->reset();

	return FuncRequest::unknown;
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


docstring KeyMap::printBindings(FuncRequest const & func,
				KeySequence::outputFormat format) const
{
	Bindings bindings = findBindings(func);
	if (bindings.empty())
		return docstring();
	
	odocstringstream res;
	Bindings::const_iterator cit = bindings.begin();
	Bindings::const_iterator cit_end = bindings.end();
	// print the first item
	res << cit->print(format);
	// more than one shortcuts?
	for (++cit; cit != cit_end; ++cit)
		res << ", " << cit->print(format);
	return res.str();
}


KeyMap::Bindings KeyMap::findBindings(FuncRequest const & func) const
{
	return findBindings(func, KeySequence(0, 0));
}


KeyMap::Bindings KeyMap::findBindings(FuncRequest const & func,
			KeySequence const & prefix) const
{
	Bindings res;
	if (table.empty())
		return res;

	Table::const_iterator end = table.end();
	for (Table::const_iterator cit = table.begin(); cit != end; ++cit) {
		if (cit->table.get()) {
			KeySequence seq = prefix;
			seq.addkey(cit->code, cit->mod.first);
			Bindings res2 = cit->table->findBindings(func, seq);
			res.insert(res.end(), res2.begin(), res2.end());
		} else if (cit->func == func) {
			KeySequence seq = prefix;
			seq.addkey(cit->code, cit->mod.first);
			res.push_back(seq);
		}
	}

	return res;
}


KeyMap::BindingList KeyMap::listBindings(bool unbound, KeyMap::ItemType tag) const
{
	BindingList list;
	listBindings(list, KeySequence(0, 0), tag);
	if (unbound) {
		LyXAction::const_func_iterator fit = lyxaction.func_begin();
		LyXAction::const_func_iterator fit_end = lyxaction.func_end();
		for (; fit != fit_end; ++fit) {
			FuncCode action = fit->second;
			bool has_action = false;
			BindingList::const_iterator it = list.begin();
			BindingList::const_iterator it_end = list.end();
			for (; it != it_end; ++it)
				if (it->request.action == action) {
					has_action = true;
					break;
				}
			if (!has_action)
				list.push_back(Binding(FuncRequest(action), KeySequence(0, 0), tag));
		}	
	}
	return list;
}


void KeyMap::listBindings(BindingList & list,
	KeySequence const & prefix, KeyMap::ItemType tag) const
{
	Table::const_iterator it = table.begin();
	Table::const_iterator it_end = table.end();
	for (; it != it_end; ++it) {
		// a LFUN_COMMAND_PREFIX
		if (it->table.get()) {
			KeySequence seq = prefix;
			seq.addkey(it->code, it->mod.first);
			it->table->listBindings(list, seq, tag);
		} else {
			KeySequence seq = prefix;
			seq.addkey(it->code, it->mod.first);
			list.push_back(Binding(it->func, seq, tag));
		}
	}
}


} // namespace lyx
