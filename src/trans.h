// -*- C++ -*-
#ifndef Trans_h
#define Trans_h

#ifdef __GNUG__
#pragma interface
#endif

#include "tex-accent.h"
#include "LString.h"
#include "trans_decl.h"

class LyXLex;

class TransManager;

/**
  TransInterface: the interface that every translation class
  should obey too.
  Visitor pattern applied here
  */
class TransInterface {
public:
	///
	virtual string const process(char, TransManager &) = 0;
	///
	virtual bool isAccentDefined(tex_accent, KmodInfo &) const = 0;
};

/**
  DefaultTrans: the default translation class. Hols info
  on tex-accents. Monostate
  */
class DefaultTrans : public TransInterface {
public:
	///
	DefaultTrans();
	///
	virtual string const process(char, TransManager &);
private:
	///
	static bool init_;
};


/**
  Trans: holds a .kmap file 
  */
class Trans : public TransInterface {
public:
	///
	Trans();
	///
	virtual ~Trans();

	///
	int Load(string const & language);
	///
	bool IsDefined() const;
	///
	string const & GetName() const;
	///
	string const process(char, TransManager &);
	///
	bool isAccentDefined(tex_accent, KmodInfo &) const;
    
private:
	///
	typedef KmodInfo kmod_list_decl;
	///
	typedef KmodException keyexc;
    
	///
	void AddDeadkey(tex_accent, string const &, string const &);
	///
	void FreeKeymap();
	///
	int Load(LyXLex &);
	///
	inline string const & Match(unsigned char c);
	///
	void InsertException(keyexc & exclist, char c,
			     string const & data, bool = false,
			     tex_accent = TEX_NOACCENT);
	///
	void FreeException(keyexc & exclist);

	///
	string name_;
	///
	string keymap_[256];
	///
	kmod_list_decl * kmod_list_[TEX_MAX_ACCENT+1];

};


///
string const & Trans::Match(unsigned char c)
{
	return keymap_[c];
}

#endif 
