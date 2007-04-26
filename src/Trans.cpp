/**
 * \file Trans.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Trans.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "Lexer.h"
#include "debug.h"
#include "TransState.h"


namespace lyx {

using support::contains;
using support::libFileSearch;

using std::endl;
using std::string;
using std::map;


// KmodInfo
KmodInfo::KmodInfo()
{
}


// Trans class

Trans::Trans()
{
}


Trans::~Trans()
{
	freeKeymap();
}


void Trans::insertException(KmodException & exclist, char_type c,
                            docstring const & data, bool flag, tex_accent accent)
{
	Keyexc p;
	p.c = c;
	p.data = data;
	p.combined = flag;
	p.accent = accent;
	exclist.insert(exclist.begin(), p);
	// or just
	// exclist.push_back(p);
}


void Trans::freeException(KmodException & exclist)
{
	exclist.clear();
}


void Trans::freeKeymap()
{
	kmod_list_.clear();
	keymap_.clear();
}


bool Trans::isDefined() const
{
	return !name_.empty();
}


string const & Trans::getName() const
{
	return name_;
}


enum kmaptags_ {
	KCOMB = 1,
	KMOD,
	KMAP,
	KXMOD,
	K_LAST
};


struct keyword_item kmapTags[K_LAST - 1] = {
	{"\\kcomb", KCOMB },
	{ "\\kmap", KMAP },
	{ "\\kmod", KMOD },
	{ "\\kxmod", KXMOD }
};


tex_accent getkeymod(string const &);


void Trans::addDeadkey(tex_accent accent, docstring const & keys)
{
	KmodInfo tmp;
	tmp.data = keys;
	tmp.accent = accent;
	kmod_list_[accent] = tmp;

	for (docstring::size_type i = 0; i < keys.length(); ++i) {
		// FIXME This is a hack.
		// tmp is no valid UCS4 string, but misused to store the
		// accent.
		docstring tmp;
		tmp += char_type(0);
		tmp += char_type(accent);
		keymap_[keys[i]] = tmp;
	}
}


int Trans::load(Lexer & lex)
{
	bool error = false;

	while (lex.isOK() && !error) {
		switch (lex.lex()) {
		case KMOD:
		{
			LYXERR(Debug::KBMAP) << "KMOD:\t" << lex.getString() << endl;

			if (lex.next(true)) {
				LYXERR(Debug::KBMAP) << "key\t`" << lex.getString()
				       << '\'' << endl;
			} else
				return -1;

			docstring const keys = lex.getDocString();

			if (lex.next(true)) {
				LYXERR(Debug::KBMAP) << "accent\t`" << lex.getString()
					       << '\'' << endl;
			} else
				return -1;

			tex_accent accent = getkeymod(lex.getString());

			if (accent == TEX_NOACCENT)
				return -1;

#if 1
//#warning This code should be removed...
			// But we need to fix up all the kmap files first
			// so that this field is not present anymore.
			if (lex.next(true)) {
				LYXERR(Debug::KBMAP) << "allowed\t`" << lex.getString()
					       << '\'' << endl;
			} else
				return -1;

			/* string const allowed = lex.getString(); */
			addDeadkey(accent, keys /*, allowed*/);
#else
			addDeadkey(accent, keys);
#endif
			break;
		}
		case KCOMB: {
			string str;

			LYXERR(Debug::KBMAP) << "KCOMB:" << endl;
			if (lex.next(true)) {
				str = lex.getString();
				LYXERR(Debug::KBMAP) << str << endl;
			} else
				return -1;

			tex_accent accent_1 = getkeymod(str);
			if (accent_1 == TEX_NOACCENT) return -1;

			if (lex.next(true)) {
				str = lex.getString();
				LYXERR(Debug::KBMAP) << str << endl;
			} else
				return -1;

			tex_accent accent_2= getkeymod(str);
			if (accent_2 == TEX_NOACCENT) return -1;

			map<tex_accent, KmodInfo>::iterator it1 =
				kmod_list_.find(accent_1);
			map<tex_accent, KmodInfo>::iterator it2 =
				kmod_list_.find(accent_2);
			if (it1 == kmod_list_.end()
			    || it2 == kmod_list_.end()) {
				return -1;
			}

			// Find what key accent_2 is on - should
			// check about accent_1 also
			map<char_type, docstring>::iterator it = keymap_.begin();
			map<char_type, docstring>::iterator end = keymap_.end();
			for (; it != end; ++it) {
				if (!it->second.empty()
				    && it->second[0] == 0
				    && it->second[1] == accent_2)
					break;
			}
			docstring allowed;
			if (lex.next()) {
				allowed = lex.getDocString();
				LYXERR(Debug::KBMAP) << "allowed: "
						     << to_utf8(allowed) << endl;
			} else {
				return -1;
			}

			insertException(kmod_list_[accent_1].exception_list,
					it->first, allowed,
					true, accent_2);
		}
		break;
		case KMAP: {
			unsigned char key_from;

			LYXERR(Debug::KBMAP) << "KMAP:\t" << lex.getString() << endl;

			if (lex.next(true)) {
				key_from = lex.getString()[0];
				LYXERR(Debug::KBMAP) << "\t`" << lex.getString() << '\''
					<< endl;
			} else
				return -1;

			if (lex.next(true)) {
				docstring const string_to = lex.getDocString();
				keymap_[key_from] = string_to;
				LYXERR(Debug::KBMAP) << "\t`" << to_utf8(string_to) << '\''
					<< endl;
			} else
				return -1;

			break;
		}
		case KXMOD: {
			tex_accent accent;
			char_type key;
			docstring str;

			LYXERR(Debug::KBMAP) << "KXMOD:\t" << lex.getString() << endl;

			if (lex.next(true)) {
				LYXERR(Debug::KBMAP) << "\t`" << lex.getString() << '\''
					<< endl;
				accent = getkeymod(lex.getString());
			} else
				return -1;

			if (lex.next(true)) {
				LYXERR(Debug::KBMAP) << "\t`" << lex.getString() << '\''
					<< endl;
				key = lex.getDocString()[0];
			} else
				return -1;

			if (lex.next(true)) {
				LYXERR(Debug::KBMAP) << "\t`" << lex.getString() << '\''
					<< endl;
				str = lex.getDocString();
			} else
				return -1;

			insertException(kmod_list_[accent].exception_list,
					key, str);
			break;
		}
		case Lexer::LEX_FEOF:
			LYXERR(Debug::PARSER) << "End of parsing" << endl;
			break;
		default:
			lex.printError("ParseKeymapFile: "
				       "Unknown tag: `$$Token'");
			return -1;
		}
	}
	return 0;
}


bool Trans::isAccentDefined(tex_accent accent, KmodInfo & i) const
{
	map<tex_accent, KmodInfo>::const_iterator cit = kmod_list_.find(accent);
	if (cit != kmod_list_.end()) {
		i = cit->second;
		return true;
	}
	return false;
}


docstring const Trans::process(char_type c, TransManager & k)
{
	docstring const t = match(c);

	if (t.empty() && c != 0) {
		return k.normalkey(c);
	} else if (!t.empty() && t[0] != 0) {
		//return k.normalkey(c);
		return t;
	} else {
		return k.deadkey(c,
				 kmod_list_[static_cast<tex_accent>(t[1])]);
	}
}


int Trans::load(string const & language)
{
	support::FileName const filename = libFileSearch("kbd", language, "kmap");
	if (filename.empty())
		return -1;

	freeKeymap();
	Lexer lex(kmapTags, K_LAST - 1);
	lex.setFile(filename);

	int const res = load(lex);

	if (res == 0) {
		name_ = language;
	} else
		name_.erase();

	return res;
}


tex_accent getkeymod(string const & p)
	/* return modifier - decoded from p and update p */
{
	for (int i = 1; i <= TEX_MAX_ACCENT; ++i) {
		LYXERR(Debug::KBMAP) << "p = " << p
		       << ", lyx_accent_table[" << i
		       << "].name = `" << lyx_accent_table[i].name
		       << '\'' << endl;

		if (lyx_accent_table[i].name
		     && contains(p, lyx_accent_table[i].name)) {
			LYXERR(Debug::KBMAP) << "Found it!" << endl;
			return static_cast<tex_accent>(i);
		}
	}
	return TEX_NOACCENT;
}


} // namespace lyx
