#include <config.h>

#include <stdio.h>

#ifdef __GNUG__
#pragma implementation "trans.h"
#endif

#include "LyXView.h"
#include "trans.h"
#include "filetools.h"
#include "tex-strings.h"
#include "lyxlex.h"
#include "error.h"
#include "trans_mgr.h"


// KmodInfo
KmodInfo::KmodInfo()
{
	exception_list=NULL;
}


KmodInfo::KmodInfo(const KmodInfo& o)
{
	(*this)=o;
}


KmodInfo& KmodInfo::operator=(const KmodInfo& o)
{
	if (this!=&o) {
		allowed=o.allowed;
		accent=o.accent;
		data=o.data;
		exception_list=o.exception_list;
	}

	return *this;
}

    
// Default Trans
bool DefaultTrans::init_=false;


DefaultTrans::DefaultTrans()
{
	if (init_==false) {
		// Do initialization
		init_=true;
	}
}


LString DefaultTrans::process(char c,TransManager& k)
{
	char dummy[2]="?";
	dummy[0]=c;
    
	return k.normalkey(c,dummy);
}


// Trans class

Trans::Trans()
{
	int i;

	for(i=0; i<256; i++)
		keymap_[i]=NULL;

	for(i=0; i<TEX_MAX_ACCENT+1; i++)
		kmod_list_[i]=0;
}


Trans::~Trans()
{
	FreeKeymap();
}


void Trans::InsertException(Trans::keyexc& exclist, char c,
			    const LString& data,bool flag,tex_accent accent)
{
	keyexc p;

	p = new Keyexc; 
	p -> next = exclist;
	p -> c = c;

	p->data = data;
	p->combined=flag;
	p->accent=accent;

	exclist = p;
}


void Trans::FreeException(Trans::keyexc& exclist)
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

	for(i=0; i<256; i++)
		if (keymap_[i]) {
			delete keymap_[i];
			keymap_[i]=NULL;
		}
	for(i=0; i<TEX_MAX_ACCENT+1; i++)
		if (kmod_list_[i]) {
			FreeException(kmod_list_[i]->exception_list);
			delete kmod_list_[i];
			kmod_list_[i]=0;
		}
}


bool Trans::IsDefined()
{
	return !name_.empty();
}


const LString& Trans::GetName()
{
	return name_;
}


enum _kmaptags {
	KCOMB=1,
	KMOD,
	KMAP,
	KXMOD,
	K_LAST
};


struct keyword_item kmapTags[K_LAST-1] = {
	{"\\kcomb",KCOMB },
	{ "\\kmap", KMAP },
	{ "\\kmod", KMOD },
	{ "\\kxmod", KXMOD }
};


tex_accent getkeymod(const LString&);


void Trans::AddDeadkey(tex_accent accent,const LString& keys,
		       const LString& allowed)
{
	if (kmod_list_[accent]) {
		FreeException(kmod_list_[accent]->exception_list);
		
		delete kmod_list_[accent];
	}
	
	kmod_list_[accent] = new kmod_list_decl;
	kmod_list_[accent]->data = keys;
	kmod_list_[accent]->accent = accent;
	if (allowed=="all") { 
		kmod_list_[accent]->allowed=lyx_accent_table[accent].native;
	} else { 
		kmod_list_[accent]->allowed = allowed;
	}
	
	for(int i=0;i<keys.length();i++) {
		char *temp;
		temp=keymap_[(unsigned char)keys[i]]=new char[2];
		temp[0]=0; temp[1]=accent;
	}
	kmod_list_[accent]->exception_list=NULL;
}


int Trans::Load(LyXLex &lex)
{
	bool error = false;

	while (lex.IsOK() && !error) {
		switch(lex.lex()) {
		case KMOD:
		{
			if (lyxerr.debugging(Error::KBMAP))
				fprintf(stderr,"KMOD:	%s\n",lex.text());
			if (lex.next(true)) {
				if (lyxerr.debugging(Error::KBMAP))
					fprintf(stderr, "key     `%s'\n", lex.text());
			} else
				return -1;
			
			LString keys = lex.GetString();

			if (lex.next(true)) {
				if ( lyxerr.debugging(Error::KBMAP))
					fprintf(stderr, "accent     `%s'\n", lex.text());
			} else
				return -1;

			tex_accent accent = getkeymod(lex.GetString());

			if (accent==TEX_NOACCENT)
				return -1;

			if (lex.next(true)) {
				if (lyxerr.debugging(Error::KBMAP))
					fprintf(stderr,
						"allowed     `%s'\n",
						lex.text());
			} else
				return -1;

			LString allowed = lex.GetString();

			AddDeadkey(accent, keys, allowed);
			break;
		}	
		case KCOMB: {
			const char *str;

			lyxerr.debug("KCOMB:",Error::KBMAP);
			if (lex.next(true)) {
				str=lex.text();
				lyxerr.debug(str,Error::KBMAP);
			} else
				return -1;
			
			tex_accent accent_1=getkeymod(str);
			if (accent_1==TEX_NOACCENT) return -1;

			if (lex.next(true)) {
				str=lex.text();
				lyxerr.debug(str,Error::KBMAP);
			} else
				return -1;

			tex_accent accent_2=getkeymod(str);
			if (accent_2==TEX_NOACCENT) return -1;

			if (kmod_list_[accent_1]==NULL || kmod_list_[accent_2]==NULL)
				return -1;

			// Find what key accent_2 is on - should check about accent_1 also
			int key;

			for(key=0;key<256;key++) 
				if (keymap_[key] && keymap_[key][0]==0 && keymap_[key][1]==accent_2)
					break;
			
			LString allowed;

			if (lex.next()) {
				allowed=lex.GetString();
				lyxerr.debug("allowed: "+allowed,Error::KBMAP);
			} else
				return -1;

			InsertException(kmod_list_[accent_1]->exception_list,(char)key,allowed,true,accent_2);
		}
		break;
		case KMAP: {
			char key_from;
			char *string_to;

			if (lyxerr.debugging(Error::KBMAP))
				fprintf(stderr, "KMAP: %s\n", lex.text());
			if (lex.next(true)) {
				key_from=lex.text()[0];
				if (lyxerr.debugging(Error::KBMAP))
					fprintf(stderr, "     `%s'\n", lex.text());
			} else
				return -1;

			if (lex.next(true)) {
				char const *t = lex.text();
				string_to = strcpy(new char[strlen(t)+1],t);
				keymap_[(unsigned char)key_from]=string_to;
				if (lyxerr.debugging(Error::KBMAP))
					fprintf(stderr, "     `%s'\n", string_to);
			} else
				return -1;

			break;
		}
		case KXMOD: {
			tex_accent accent;
			char key;
			const char *str;

			if (lyxerr.debugging(Error::KBMAP))
				fprintf(stderr, "KXMOD: %s\n", lex.text());
			if (lex.next(true)) {
				if (lyxerr.debugging(Error::KBMAP))
					fprintf(stderr, "     `%s'\n", lex.text());
				accent = getkeymod(lex.GetString());
			} else
				return -1;

			if (lex.next(true)) {
				if (lyxerr.debugging(Error::KBMAP))
					fprintf(stderr, "      `%s'\n", lex.text());
				key=lex.text()[0];
			} else
				return -1;

			if (lex.next(true)) {
				if (lyxerr.debugging(Error::KBMAP))
					fprintf(stderr, "      `%s'\n", lex.text());
				str=lex.text();
			} else
				return -1;

			InsertException(kmod_list_[accent]->exception_list,key,str);
			break;
		}
		case LyXLex::LEX_FEOF:
			lyxerr.debug("End of parsing",Error::LEX_PARSER);
			break;
		default:
			lex.printError("ParseKeymapFile: "
				       "Unknown tag: `$$Token'");
			return -1;
		}
	}
	return 0;
}


bool Trans::isAccentDefined(tex_accent accent,KmodInfo& i)
{
	if (kmod_list_[accent]!=NULL) {
		i=*kmod_list_[accent];
		return true;
	}
	return false;
}


LString Trans::process(char c,TransManager& k)
{
	char dummy[2]="?";
	char *dt=dummy;
	char *t=Match(c);
	
    
	if ((t==NULL && (*dt=c)) || (t[0]!=0 && (dt=t)) ){
		return k.normalkey(c,dt);
	} else {
		return k.deadkey(c,*kmod_list_[(tex_accent)t[1]]);
	}
}


int Trans::Load(LString const &language)
{
	LString filename = LibFileSearch("kbd", language, "kmap");
	if (filename.empty())
		return -1;

	FreeKeymap();
	LyXLex lex(kmapTags, K_LAST-1);
	lex.setFile(filename);
	
	int res=Load(lex);

	if (res==0) {
		name_=language;
	} else
		name_.clean();

	return res;
}


tex_accent getkeymod(LString const &p)
	/* return modifier - decoded from p and update p */
{
	for (int i = 1; i <= TEX_MAX_ACCENT; i++) {
		if (lyxerr.debugging(Error::KBMAP))
			fprintf(stderr,
				"p = %s, lyx_accent_table[%d].name = `%s'\n",
				p.c_str(), i, lyx_accent_table[i].name);
		
		if ( lyx_accent_table[i].name && p.contains(lyx_accent_table[i].name)) {
			lyxerr.debug("Found it!",Error::KBMAP);
			return (tex_accent)i;
		}
	}
	return TEX_NOACCENT;
}
