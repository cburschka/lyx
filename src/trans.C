#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LyXView.h"
#include "trans.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "tex-strings.h"
#include "lyxlex.h"
#include "debug.h"
#include "trans_mgr.h"

using std::endl;


// KmodInfo
KmodInfo::KmodInfo()
{
#if 0
	exception_list = 0;
#endif
}


#if 0
// Default Trans
bool DefaultTrans::init_ = false;


DefaultTrans::DefaultTrans()
{
	if (init_ == false) {
		// Do initialization
		init_ = true;
	}
}


#if 0
string const DefaultTrans::process(char c, TransManager & k)
{
	char dummy[2] = "?";
	dummy[0] = c;
    
	return k.normalkey(c, dummy);
}
#else
string const DefaultTrans::process(char c, TransManager & k)
{
	return k.normalkey(c);
}
#endif
#endif

// Trans class

Trans::Trans()
{
#if 0
	for (int i = 0; i < TEX_MAX_ACCENT + 1; ++i)
		kmod_list_[i] = 0;
#endif
}


Trans::~Trans()
{
	FreeKeymap();
}


void Trans::InsertException(KmodException & exclist, char c,
			    string const & data, bool flag, tex_accent accent)
{
#if 0
	keyexc p = new Keyexc; 
	p->next = exclist;
	p->c = c;

	p->data = data;
	p->combined = flag;
	p->accent = accent;

	exclist = p;
#else
	Keyexc p;
	p.c = c;
	p.data = data;
	p.combined = flag;
	p.accent = accent;
	exclist.insert(exclist.begin(), p);
	// or just
	// exclist.push_back(p);
#endif
}


void Trans::FreeException(KmodException & exclist)
{
#if 0
	Trans::keyexc p = exclist;
	while (p) {
		p = exclist->next;
		delete exclist;
		exclist = p;
	}
#else
	exclist.clear();
#endif
}


void Trans::FreeKeymap()
{
#if 0
	for (int i = 0; i < 256; ++i)
		if (!keymap_[i].empty()) {
			keymap_[i].erase();
		}
	for (int i = 0; i < TEX_MAX_ACCENT + 1; ++i)
		if (kmod_list_[i]) {
			FreeException(kmod_list_[i]->exception_list);
			delete kmod_list_[i];
			kmod_list_[i] = 0;
		}
#else
	kmod_list_.clear();
	keymap_.clear();
#endif
}


bool Trans::IsDefined() const
{
	return !name_.empty();
}


string const & Trans::GetName() const
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


#if 0
void Trans::AddDeadkey(tex_accent accent, string const & keys,
		       string const & allowed)
#else
void Trans::AddDeadkey(tex_accent accent, string const & keys)
#endif
{
#if 0
	if (kmod_list_[accent]) {
		FreeException(kmod_list_[accent]->exception_list);
		
		delete kmod_list_[accent];
	}
	
	kmod_list_[accent] = new KmodInfo;
	kmod_list_[accent]->data = keys;
	kmod_list_[accent]->accent = accent;
#else
	KmodInfo tmp;
	tmp.data = keys;
	tmp.accent = accent;
	kmod_list_[accent] = tmp;
#endif
#if 0
	if (allowed == "native") { 
		kmod_list_[accent]->allowed= lyx_accent_table[accent].native;
	} else {
		kmod_list_[accent]->allowed = allowed;
	}

	for (string::size_type i = 0; i < keys.length(); ++i) {
		string & temp =
			keymap_[static_cast<unsigned char>(keys[i])];
		if (!temp.empty())
			temp.erase();

		// But the question remains: "Should we be allowed
		// to change bindings, without unbinding first?"
		// Lgb
		temp += char(0);
		temp += char(accent);
	}
#else
	for (string::size_type i = 0; i < keys.length(); ++i) {
		string tmp;
		tmp += char(0);
		tmp += char(accent);
		keymap_[keys[i]] = tmp;
	}
#endif
#if 0
	kmod_list_[accent]->exception_list = 0;
#endif
}


int Trans::Load(LyXLex & lex)
{
	bool error = false;

	while (lex.isOK() && !error) {
		switch (lex.lex()) {
		case KMOD:
		{
			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "KMOD:\t" << lex.text() << endl;
			if (lex.next(true)) {
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "key\t`" << lex.text()
					       << "'" << endl;
			} else
				return -1;
			
			string const keys = lex.getString();

			if (lex.next(true)) {
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "accent\t`" << lex.text()
					       << "'" << endl;
			} else
				return -1;

			tex_accent accent = getkeymod(lex.getString());

			if (accent == TEX_NOACCENT)
				return -1;

#if 1
#warning This code should be removed...
			// But we need to fix up all the kmap files first
			// so that this field is not present anymore.
			if (lex.next(true)) {
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "allowed\t`" << lex.text()
					       << "'" << endl;
			} else
				return -1;

			string const allowed = lex.getString();
			AddDeadkey(accent, keys /*, allowed*/);
#else
			AddDeadkey(accent, keys);
#endif
			break;
		}	
		case KCOMB: {
			string str;

			lyxerr[Debug::KBMAP] << "KCOMB:" << endl;
			if (lex.next(true)) {
				str= lex.text();
				lyxerr[Debug::KBMAP] << str << endl;
			} else
				return -1;
			
			tex_accent accent_1 = getkeymod(str);
			if (accent_1 == TEX_NOACCENT) return -1;

			if (lex.next(true)) {
				str = lex.text();
				lyxerr[Debug::KBMAP] << str << endl;
			} else
				return -1;

			tex_accent accent_2= getkeymod(str);
			if (accent_2 == TEX_NOACCENT) return -1;

#if 0
			if (kmod_list_[accent_1] == 0
			    || kmod_list_[accent_2] == 0)
				return -1;
#else
			std::map<int, KmodInfo>::iterator it1 =
				kmod_list_.find(accent_1);
			std::map<int, KmodInfo>::iterator it2 =
				kmod_list_.find(accent_2);
			if (it1 == kmod_list_.end()
			    || it2 == kmod_list_.end()) {
				return -1;
			}
#endif

			// Find what key accent_2 is on - should
			// check about accent_1 also
#if 0
			int key = 0;
			for (; key < 256; ++key) {
				if (!keymap_[key].empty()
				    && keymap_[key][0] == 0
				    && keymap_[key][1] == accent_2)
					break;
			}
			string allowed;

			if (lex.next()) {
				allowed = lex.GetString();
				lyxerr[Debug::KBMAP] << "allowed: "
						     << allowed << endl;
			} else
				return -1;

			InsertException(kmod_list_[accent_1].exception_list,
					static_cast<char>(key), allowed,
					true, accent_2);
#else
			std::map<int, string>::iterator it = keymap_.begin();
			std::map<int, string>::iterator end = keymap_.end();
			for (; it != end; ++it) {
				if (!it->second.empty()
				    && it->second[0] == 0
				    && it->second[1] == accent_2)
					break;
			}
			string allowed;
			if (lex.next()) {
				allowed = lex.getString();
				lyxerr[Debug::KBMAP] << "allowed: "
						     << allowed << endl;
			} else {
				return -1;
			}
			
			InsertException(kmod_list_[accent_1].exception_list,
					static_cast<char>(it->first), allowed,
					true, accent_2);
#endif
		}
		break;
		case KMAP: {
			unsigned char key_from;

			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "KMAP:\t" << lex.text() << endl;
			if (lex.next(true)) {
				key_from = lex.text()[0];
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "\t`" << lex.text() << "'"
					       << endl;
			} else
				return -1;

			if (lex.next(true)) {
				string string_to = lex.text();
				keymap_[key_from] = string_to;
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "\t`" << string_to << "'"
					       << endl;
			} else
				return -1;

			break;
		}
		case KXMOD: {
			tex_accent accent;
			char key;
			string str;

			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "KXMOD:\t" << lex.text() << endl;
			if (lex.next(true)) {
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "\t`" << lex.text() << "'"
					       << endl;
				accent = getkeymod(lex.getString());
			} else
				return -1;

			if (lex.next(true)) {
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "\t`" << lex.text() << "'"
					       << endl;
				key = lex.text()[0];
			} else
				return -1;

			if (lex.next(true)) {
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "\t`" << lex.text() << "'"
					       << endl;
				str = lex.text();
			} else
				return -1;

			InsertException(kmod_list_[accent].exception_list,
					key, str);
			break;
		}
		case LyXLex::LEX_FEOF:
			lyxerr[Debug::PARSER] << "End of parsing" << endl;
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
#if 0
	if (kmod_list_[accent] != 0) {
		i = *kmod_list_[accent];
		return true;
	}
	return false;
#else
	std::map<int, KmodInfo>::const_iterator cit = kmod_list_.find(accent);
	if (cit != kmod_list_.end()) {
		i = cit->second;
		return true;
	}
	return false;
#endif
}


#if 0
string const Trans::process(char c, TransManager & k)
{
	string dt("?");
	string const t = Match(static_cast<unsigned char>(c));

	if (t.empty() && c != 0) {
		dt[0] = c;
		return k.normalkey(c, dt);
	} else if (!t.empty() && t[0] != char(0)) {
		dt = t;
		return k.normalkey(c, dt);
	} else {
		return k.deadkey(c,
				 *kmod_list_[static_cast<tex_accent>(t[1])]);
	}
}
#else
string const Trans::process(char c, TransManager & k)
{
	string const t = Match(static_cast<unsigned char>(c));

	if (t.empty() && c != 0) {
		return k.normalkey(c);
	} else if (!t.empty() && t[0] != char(0)) {
		//return k.normalkey(c);
		return t;
	} else {
#if 0
		return k.deadkey(c,
				 *kmod_list_[static_cast<tex_accent>(t[1])]);
#else
		return k.deadkey(c,
				 kmod_list_[static_cast<tex_accent>(t[1])]);
#endif
	}
}
#endif


int Trans::Load(string const & language)
{
	string const filename = LibFileSearch("kbd", language, "kmap");
	if (filename.empty())
		return -1;

	FreeKeymap();
	LyXLex lex(kmapTags, K_LAST-1);
	lex.setFile(filename);
	
	int const res = Load(lex);

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
		if (lyxerr.debugging(Debug::KBMAP))
			lyxerr << "p = " << p
			       << ", lyx_accent_table[" << i
			       << "].name = `" << lyx_accent_table[i].name
			       << "'" << endl;
		
		if (lyx_accent_table[i].name
		     && contains(p, lyx_accent_table[i].name)) {
			lyxerr[Debug::KBMAP] << "Found it!" << endl;
			return static_cast<tex_accent>(i);
		}
	}
	return TEX_NOACCENT;
}
