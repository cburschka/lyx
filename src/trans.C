#include <config.h>

#ifdef __GNUG__
#pragma implementation "trans.h"
#endif

#include "LyXView.h"
#include "trans.h"
#include "support/filetools.h"
#include "tex-strings.h"
#include "lyxlex.h"
#include "debug.h"
#include "trans_mgr.h"


// KmodInfo
KmodInfo::KmodInfo()
{
	exception_list = 0;
}


// Default Trans
bool DefaultTrans::init_ = false;


DefaultTrans::DefaultTrans()
{
	if (init_ == false) {
		// Do initialization
		init_ = true;
	}
}


string DefaultTrans::process(char c, TransManager & k)
{
	char dummy[2] = "?";
	dummy[0] = c;
    
	return k.normalkey(c, dummy);
}


// Trans class

Trans::Trans()
{
	int i;

	for(i = 0; i < 256; ++i)
		keymap_[i] = 0;

	for(i = 0; i < TEX_MAX_ACCENT + 1; ++i)
		kmod_list_[i] = 0;
}


Trans::~Trans()
{
	FreeKeymap();
}


void Trans::InsertException(Trans::keyexc & exclist, char c,
			    string const & data, bool flag, tex_accent accent)
{
	keyexc p;

	p = new Keyexc; 
	p->next = exclist;
	p->c = c;

	p->data = data;
	p->combined = flag;
	p->accent = accent;

	exclist = p;
}


void Trans::FreeException(Trans::keyexc & exclist)
{
	Trans::keyexc p;

	p = exclist;
	while (p) {
		p = exclist->next;
		delete exclist;
		exclist = p;
	}
}


void Trans::FreeKeymap()
{
	int i;

	for(i = 0; i < 256; ++i)
		if (keymap_[i]) {
			delete keymap_[i];
			keymap_[i] = 0;
		}
	for(i = 0; i < TEX_MAX_ACCENT + 1; ++i)
		if (kmod_list_[i]) {
			FreeException(kmod_list_[i]->exception_list);
			delete kmod_list_[i];
			kmod_list_[i] = 0;
		}
}


bool Trans::IsDefined()
{
	return !name_.empty();
}


string const & Trans::GetName()
{
	return name_;
}


enum _kmaptags {
	KCOMB = 1,
	KMOD,
	KMAP,
	KXMOD,
	K_LAST
};


struct keyword_item kmapTags[K_LAST-1] = {
	{"\\kcomb", KCOMB },
	{ "\\kmap", KMAP },
	{ "\\kmod", KMOD },
	{ "\\kxmod", KXMOD }
};


tex_accent getkeymod(string const &);


void Trans::AddDeadkey(tex_accent accent, string const & keys,
		       string const & allowed)
{
	if (kmod_list_[accent]) {
		FreeException(kmod_list_[accent]->exception_list);
		
		delete kmod_list_[accent];
	}
	
	kmod_list_[accent] = new kmod_list_decl;
	kmod_list_[accent]->data = keys;
	kmod_list_[accent]->accent = accent;
	if (allowed == "all") { 
		kmod_list_[accent]->allowed= lyx_accent_table[accent].native;
	} else { 
		kmod_list_[accent]->allowed = allowed;
	}
	
	for(string::size_type i = 0; i < keys.length(); ++i) {
		char * temp;
		if (keys[i] == char(254))
			lyxerr << "check one triggered" << endl;
		temp = keymap_[keys[i]] = new char[2];
		temp[0] = 0; temp[1] = accent;
	}
	kmod_list_[accent]->exception_list = 0;
}


int Trans::Load(LyXLex & lex)
{
	bool error = false;

	while (lex.IsOK() && !error) {
		switch(lex.lex()) {
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
			
			string keys = lex.GetString();

			if (lex.next(true)) {
				if ( lyxerr.debugging(Debug::KBMAP))
					lyxerr << "accent\t`" << lex.text()
					       << "'" << endl;
			} else
				return -1;

			tex_accent accent = getkeymod(lex.GetString());

			if (accent == TEX_NOACCENT)
				return -1;

			if (lex.next(true)) {
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "allowed\t`" << lex.text()
					       << "'" << endl;
			} else
				return -1;

			string allowed = lex.GetString();

			AddDeadkey(accent, keys, allowed);
			break;
		}	
		case KCOMB: {
			char const * str;

			lyxerr[Debug::KBMAP] << "KCOMB:" << endl;
			if (lex.next(true)) {
				str= lex.text();
				lyxerr[Debug::KBMAP] << str << endl;
			} else
				return -1;
			
			tex_accent accent_1= getkeymod(str);
			if (accent_1 == TEX_NOACCENT) return -1;

			if (lex.next(true)) {
				str= lex.text();
				lyxerr[Debug::KBMAP] << str << endl;
			} else
				return -1;

			tex_accent accent_2= getkeymod(str);
			if (accent_2 == TEX_NOACCENT) return -1;

			if (kmod_list_[accent_1] == 0 || kmod_list_[accent_2] == 0)
				return -1;

			// Find what key accent_2 is on - should check about accent_1 also
			int key;

			for(key = 0; key < 256; ++key) {
				if (keymap_[key] && keymap_[key][0] == 0
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

			InsertException(kmod_list_[accent_1]->exception_list,(char)key, allowed, true, accent_2);
		}
		break;
		case KMAP: {
			char key_from;
			char * string_to;

			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "KMAP:\t" << lex.text() << endl;
			if (lex.next(true)) {
				key_from= lex.text()[0];
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "\t`" << lex.text() << "'"
					       << endl;
			} else
				return -1;

			if (lex.next(true)) {
				char const * t = lex.text();
				string_to = strcpy(new char[strlen(t)+1], t);
				if (key_from == char(254))
					lyxerr << "check two triggered"
					       << endl;
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
			char const * str;

			if (lyxerr.debugging(Debug::KBMAP))
				lyxerr << "KXMOD:\t" << lex.text() << endl;
			if (lex.next(true)) {
				if (lyxerr.debugging(Debug::KBMAP))
					lyxerr << "\t`" << lex.text() << "'"
					       << endl;
				accent = getkeymod(lex.GetString());
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

			InsertException(kmod_list_[accent]->exception_list, key, str);
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


bool Trans::isAccentDefined(tex_accent accent, KmodInfo & i)
{
	if (kmod_list_[accent]!= 0) {
		i = *kmod_list_[accent];
		return true;
	}
	return false;
}


string Trans::process(char c, TransManager & k)
{
	lyxerr << "enter process" << endl;
	char dummy[2] = "?";
	char * dt = dummy;
	char * t = Match(c);
	lyxerr << "finished variables" << endl;
    
	if ((t == 0 && (dt[0] = c)) || (t[0] != 0 && (dt = t)) ){
		lyxerr << "normalkey" << endl;
		return k.normalkey(c, dt);
	} else {
		lyxerr << "deadkey" << endl;
		return k.deadkey(c, *kmod_list_[(tex_accent)t[1]]);
	}
}


int Trans::Load(string const & language)
{
	string filename = LibFileSearch("kbd", language, "kmap");
	if (filename.empty())
		return -1;

	FreeKeymap();
	LyXLex lex(kmapTags, K_LAST-1);
	lex.setFile(filename);
	
	int res = Load(lex);

	if (res == 0) {
		name_ = language;
	} else
		name_.clear();

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
		
		if ( lyx_accent_table[i].name && contains(p, lyx_accent_table[i].name)) {
			lyxerr[Debug::KBMAP] << "Found it!" << endl;
			return (tex_accent)i;
		}
	}
	return TEX_NOACCENT;
}
