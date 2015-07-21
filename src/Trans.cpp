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

#include "Buffer.h"
#include "BufferView.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "Text.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// TeXAccents
//
/////////////////////////////////////////////////////////////////////

/* the names used by TeX and XWindows for deadkeys/accents are not the same
   so here follows a table to clearify the differences. Please correct this
   if I got it wrong

   |------------------|------------------|------------------|--------------|
   |      TeX         |     XWindows     |   \bind/LFUN     | used by intl |
   |------------------|------------------|------------------|--------------|
   |    grave         |    grave         |LFUN_ACCENT_GRAVE        | grave
   |    acute         |    acute         |LFUN_ACCENT_ACUTE        | acute
   |    circumflex    |    circumflex    |LFUN_ACCENT_CIRCUMFLEX   | circumflex
   | umlaut/dieresis  |    diaeresis     |LFUN_ACCENT_UMLAUT       | umlaut
   |    tilde         |    tilde         |LFUN_ACCENT_TILDE        | tilde
   |    macron        |    maron         |LFUN_ACCENT_MACRON       | macron
   |    dot           |    abovedot      |LFUN_ACCENT_DOT          | dot
   |    cedilla       |    cedilla       |LFUN_ACCENT_CEDILLA      | cedilla
   |    underdot      |                  |LFUN_ACCENT_UNDERDOT     | underdot
   |    underbar      |                  |LFUN_ACCENT_UNDERBAR     | underbar
   |    hácek         |    caron         |LFUN_ACCENT_CARON        | caron
   |    breve         |    breve         |LFUN_ACCENT_BREVE        | breve
   |    tie           |                  |LFUN_ACCENT_TIE          | tie
   | Hungarian umlaut |    doubleacute   |LFUN_ACCENT_HUNGARIAN_UMLAUT  | hungarian umlaut
   |    circle        |    abovering     |LFUN_ACCENT_CIRCLE       | circle
   |                  |    ogonek        |                  |
   |                  |    iota          |                  |
   |                  |    voiced_sound  |                  |
   |                  | semivoiced_sound |                  |
   */
static TeXAccent lyx_accent_table[] = {
	{TEX_NOACCENT,   0,      "",                LFUN_NOACTION},
	{TEX_ACUTE,      0x0301, "acute",           LFUN_ACCENT_ACUTE},
	{TEX_GRAVE,      0x0300, "grave",           LFUN_ACCENT_GRAVE},
	{TEX_MACRON,     0x0304, "macron",          LFUN_ACCENT_MACRON},
	{TEX_TILDE,      0x0303, "tilde",           LFUN_ACCENT_TILDE},
	{TEX_UNDERBAR,   0x0320, "underbar",        LFUN_ACCENT_UNDERBAR},
	{TEX_CEDILLA,    0x0327, "cedilla",         LFUN_ACCENT_CEDILLA},
	{TEX_UNDERDOT,   0x0323, "underdot",        LFUN_ACCENT_UNDERDOT},
	{TEX_CIRCUMFLEX, 0x0302, "circumflex",      LFUN_ACCENT_CIRCUMFLEX},
	{TEX_CIRCLE,     0x030a, "circle",          LFUN_ACCENT_CIRCLE},
	{TEX_TIE,        0x0361, "tie",             LFUN_ACCENT_TIE},
	{TEX_BREVE,      0x0306, "breve",           LFUN_ACCENT_BREVE},
	{TEX_CARON,      0x030c, "caron",           LFUN_ACCENT_CARON},
	// Don't fix this typo for compatibility reasons!
	{TEX_HUNGUML,    0x030b, "hugarian_umlaut", LFUN_ACCENT_HUNGARIAN_UMLAUT},
	{TEX_UMLAUT,     0x0308, "umlaut",          LFUN_ACCENT_UMLAUT},
	{TEX_DOT,        0x0307, "dot",             LFUN_ACCENT_DOT},
	{TEX_OGONEK,     0x0328, "ogonek",          LFUN_ACCENT_OGONEK}
};


TeXAccent get_accent(FuncCode action)
{
	int i = 0;
	while (i <= TEX_MAX_ACCENT) {
		if (lyx_accent_table[i].action == action)
			return lyx_accent_table[i];
		++i;
	}
	struct TeXAccent temp = { static_cast<tex_accent>(0), 0,
					  0, static_cast<FuncCode>(0)};
	return temp;
}


static docstring const doAccent(docstring const & s, tex_accent accent)
{
	if (s.empty())
		return docstring(1, lyx_accent_table[accent].ucs4);

	odocstringstream os;
	os.put(s[0]);
	os.put(lyx_accent_table[accent].ucs4);
	if (s.length() > 1) {
		if (accent != TEX_TIE || s.length() > 2)
			lyxerr << "Warning: Too many characters given for accent "
			       << lyx_accent_table[accent].name << '.' << endl;
		os << s.substr(1);
	}
	return normalize_c(os.str());
}


static docstring const doAccent(char_type c, tex_accent accent)
{
	return doAccent(docstring(1, c), accent);
}



/////////////////////////////////////////////////////////////////////
//
// Trans
//
/////////////////////////////////////////////////////////////////////


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


enum {
	KCOMB = 1,
	KMOD,
	KMAP,
	KXMOD
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
			LYXERR(Debug::KBMAP, "KMOD:\t" << lex.getString());
			if (!lex.next(true))
				return -1;

			LYXERR(Debug::KBMAP, "key\t`" << lex.getString() << '\'');

			docstring const keys = lex.getDocString();

			if (!lex.next(true))
				return -1;

			LYXERR(Debug::KBMAP, "accent\t`" << lex.getString() << '\'');

			tex_accent accent = getkeymod(lex.getString());

			if (accent == TEX_NOACCENT)
				return -1;

#if 1
			// FIXME: This code should be removed...
			// But we need to fix up all the kmap files first
			// so that this field is not present anymore.
			if (!lex.next(true))
				return -1;

			LYXERR(Debug::KBMAP, "allowed\t`" << lex.getString() << '\'');

			/* string const allowed = lex.getString(); */
			addDeadkey(accent, keys /*, allowed*/);
#else
			addDeadkey(accent, keys);
#endif
			break;
		}
		case KCOMB: {
			string str;

			LYXERR(Debug::KBMAP, "KCOMB:");
			if (!lex.next(true))
				return -1;

			str = lex.getString();
			LYXERR(Debug::KBMAP, str);

			tex_accent accent_1 = getkeymod(str);
			if (accent_1 == TEX_NOACCENT)
				return -1;

			if (!lex.next(true))
				return -1;

			str = lex.getString();
			LYXERR(Debug::KBMAP, str);

			tex_accent accent_2 = getkeymod(str);
			if (accent_2 == TEX_NOACCENT) return -1;

			map<tex_accent, KmodInfo>::iterator it1 =
				kmod_list_.find(accent_1);
			map<tex_accent, KmodInfo>::iterator it2 =
				kmod_list_.find(accent_2);
			if (it1 == kmod_list_.end() || it2 == kmod_list_.end())
				return -1;

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
			if (!lex.next())
				return -1;

			allowed = lex.getDocString();
			LYXERR(Debug::KBMAP, "allowed: " << to_utf8(allowed));

			insertException(kmod_list_[accent_1].exception_list,
					it->first, allowed, true, accent_2);
		}
		break;
		case KMAP: {
			unsigned char key_from;

			LYXERR(Debug::KBMAP, "KMAP:\t" << lex.getString());

			if (!lex.next(true))
				return -1;

			key_from = lex.getString()[0];
			LYXERR(Debug::KBMAP, "\t`" << lex.getString() << '\'');

			if (!lex.next(true))
				return -1;

			docstring const string_to = lex.getDocString();
			keymap_[key_from] = string_to;
			LYXERR(Debug::KBMAP, "\t`" << to_utf8(string_to) << '\'');
			break;
		}
		case KXMOD: {
			tex_accent accent;
			char_type key;
			docstring str;

			LYXERR(Debug::KBMAP, "KXMOD:\t" << lex.getString());

			if (!lex.next(true))
				return -1;

			LYXERR(Debug::KBMAP, "\t`" << lex.getString() << '\'');
			accent = getkeymod(lex.getString());

			if (!lex.next(true))
				return -1;

			LYXERR(Debug::KBMAP, "\t`" << lex.getString() << '\'');
			key = lex.getDocString()[0];

			if (!lex.next(true))
				return -1;

			LYXERR(Debug::KBMAP, "\t`" << lex.getString() << '\'');
			str = lex.getDocString();

			insertException(kmod_list_[accent].exception_list,
					key, str);
			break;
		}
		case Lexer::LEX_FEOF:
			LYXERR(Debug::PARSER, "End of parsing");
			break;
		default:
			lex.printError("ParseKeymapFile: Unknown tag: `$$Token'");
			return -1;
		}
	}
	return 0;
}


bool Trans::isAccentDefined(tex_accent accent, KmodInfo & i) const
{
	map<tex_accent, KmodInfo>::const_iterator cit = kmod_list_.find(accent);
	if (cit == kmod_list_.end())
		return false;
	i = cit->second;
	return true;
}


docstring const Trans::process(char_type c, TransManager & k)
{
	docstring const t = match(c);

	if (t.empty() && c != 0)
		return k.normalkey(c);

	if (!t.empty() && t[0] != 0)
		return t; //return k.normalkey(c);

	return k.deadkey(c, kmod_list_[static_cast<tex_accent>(t[1])]);
}


int Trans::load(string const & language)
{
	LexerKeyword kmapTags[] = {
		{"\\kcomb", KCOMB },
		{ "\\kmap", KMAP },
		{ "\\kmod", KMOD },
		{ "\\kxmod", KXMOD }
	};

	FileName const filename = libFileSearch("kbd", language, "kmap");
	if (filename.empty())
		return -1;

	freeKeymap();
	Lexer lex(kmapTags);
	lex.setFile(filename);

	int const res = load(lex);

	if (res == 0)
		name_ = language;
	else
		name_.erase();

	return res;
}


tex_accent getkeymod(string const & p)
	/* return modifier - decoded from p and update p */
{
	for (int i = 1; i <= TEX_MAX_ACCENT; ++i) {
		LYXERR(Debug::KBMAP, "p = " << p
		       << ", lyx_accent_table[" << i
		       << "].name = `" << lyx_accent_table[i].name << '\'');

		if (lyx_accent_table[i].name
		     && contains(p, lyx_accent_table[i].name)) {
			LYXERR(Debug::KBMAP, "Found it!");
			return static_cast<tex_accent>(i);
		}
	}
	return TEX_NOACCENT;
}


/////////////////////////////////////////////////////////////////////
//
// TransState
//
/////////////////////////////////////////////////////////////////////


// TransFSMData
TransFSMData::TransFSMData()
{
	deadkey_ = deadkey2_ = 0;
	deadkey_info_.accent = deadkey2_info_.accent = TEX_NOACCENT;
}


// TransState
char_type const TransState::TOKEN_SEP = 4;


// TransInitState
TransInitState::TransInitState()
{
	init_state_ = this;
}


docstring const TransInitState::normalkey(char_type c)
{
	docstring res;
	res = c;
	return res;
}


docstring const TransInitState::deadkey(char_type c, KmodInfo d)
{
	deadkey_ = c;
	deadkey_info_ = d;
	currentState = deadkey_state_;
	return docstring();
}


// TransDeadkeyState
TransDeadkeyState::TransDeadkeyState()
{
	deadkey_state_ = this;
}


docstring const TransDeadkeyState::normalkey(char_type c)
{
	docstring res;

	KmodException::iterator it = deadkey_info_.exception_list.begin();
	KmodException::iterator end = deadkey_info_.exception_list.end();

	for (; it != end; ++it) {
		if (it->c == c) {
			res = it->data;
			break;
		}
	}
	if (it == end) {
		res = doAccent(c, deadkey_info_.accent);
	}
	currentState = init_state_;
	return res;
}


docstring const TransDeadkeyState::deadkey(char_type c, KmodInfo d)
{
	docstring res;

	// Check if the same deadkey was typed twice
	if (deadkey_ == c) {
		res = deadkey_;
		deadkey_ = 0;
		deadkey_info_.accent = TEX_NOACCENT;
		currentState = init_state_;
		return res;
	}

	// Check if it is a combination or an exception
	KmodException::const_iterator cit = deadkey_info_.exception_list.begin();
	KmodException::const_iterator end = deadkey_info_.exception_list.end();
	for (; cit != end; ++cit) {
		if (cit->combined == true && cit->accent == d.accent) {
			deadkey2_ = c;
			deadkey2_info_ = d;
			comb_info_ = (*cit);
			currentState = combined_state_;
			return docstring();
		}
		if (cit->c == c) {
			res = cit->data;
			deadkey_ = 0;
			deadkey_info_.accent = TEX_NOACCENT;
			currentState = init_state_;
			return res;
		}
	}

	// Not a combination or an exception.
	// Output deadkey1 and keep deadkey2

	if (deadkey_!= 0)
		res = deadkey_;
	deadkey_ = c;
	deadkey_info_ = d;
	currentState = deadkey_state_;
	return res;
}


TransCombinedState::TransCombinedState()
{
	combined_state_ = this;
}


docstring const TransCombinedState::normalkey(char_type c)
{
	docstring const temp = doAccent(c, deadkey2_info_.accent);
	docstring const res = doAccent(temp, deadkey_info_.accent);
	currentState = init_state_;
	return res;
}


docstring const TransCombinedState::deadkey(char_type c, KmodInfo d)
{
	// Third key in a row. Output the first one and
	// reenter with shifted deadkeys
	docstring res;
	if (deadkey_ != 0)
		res = deadkey_;
	res += TOKEN_SEP;
	deadkey_ = deadkey2_;
	deadkey_info_ = deadkey2_info_;
	res += deadkey_state_->deadkey(c, d);
	return res;
}


// TransFSM
TransFSM::TransFSM()
	: TransFSMData(), TransInitState(), TransDeadkeyState(), TransCombinedState()
{
	currentState = init_state_;
}


// TransManager

// Initialize static member.
Trans TransManager::default_;


TransManager::TransManager()
	: active_(&default_)
{}


int TransManager::setPrimary(string const & language)
{
	if (t1_.getName() == language)
		return 0;

	return t1_.load(language);
}


int TransManager::setSecondary(string const & language)
{
	if (t2_.getName() == language)
		return 0;

	return t2_.load(language);
}


void TransManager::enablePrimary()
{
	if (t1_.isDefined())
		active_ = &t1_;

	LYXERR(Debug::KBMAP, "Enabling primary keymap");
}


void TransManager::enableSecondary()
{
	if (t2_.isDefined())
		active_ = &t2_;
	LYXERR(Debug::KBMAP, "Enabling secondary keymap");
}


void TransManager::disableKeymap()
{
	active_ = &default_;
	LYXERR(Debug::KBMAP, "Disabling keymap");
}


void  TransManager::translateAndInsert(char_type c, Text * text, Cursor & cur)
{
	docstring res = active_->process(c, *this);

	// Process with tokens
	docstring temp;

	while (res.length() > 0) {
		res = split(res, temp, TransState::TOKEN_SEP);
		insert(temp, text, cur);
	}
}


void TransManager::insert(docstring const & str, Text * text, Cursor & cur)
{
	for (size_t i = 0, n = str.size(); i != n; ++i)
		text->insertChar(cur, str[i]);
}


void TransManager::deadkey(char_type c, tex_accent accent, Text * t, Cursor & cur)
{
	if (c == 0 && active_ != &default_) {
		// A deadkey was pressed that cannot be printed
		// or a accent command was typed in the minibuffer
		KmodInfo i;
		if (active_->isAccentDefined(accent, i) == true) {
			docstring const res = trans_fsm_
				.currentState->deadkey(c, i);
			insert(res, t, cur);
			return;
		}
	}

	if (active_ == &default_ || c == 0) {
		KmodInfo i;
		i.accent = accent;
		i.data.erase();
		docstring res = trans_fsm_.currentState->deadkey(c, i);
		insert(res, t, cur);
	} else {
		// Go through the translation
		translateAndInsert(c, t, cur);
	}
}


} // namespace lyx
