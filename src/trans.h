// -*- C++ -*-
#ifndef Trans_h
#define Trans_h

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "tex-accent.h"
#include "LString.h"
#include "trans_decl.h"

class LyXLex;

class TransManager;

#if 0
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
  DefaultTrans: the default translation class. Holds info
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
#endif

/**
  Trans: holds a .kmap file 
  */
//class Trans : public TransInterface {
class Trans {
public:
	///
	Trans();
	///
	~Trans();

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
#if 0
	///
	typedef KmodInfo kmod_list_decl;
	///
	typedef KmodException keyexc;
#endif
#if 0
	///
	void AddDeadkey(tex_accent, string const &, string const &);
#else
	///
	void AddDeadkey(tex_accent, string const &);
#endif
	///
	void FreeKeymap();
	///
	int Load(LyXLex &);
	///
	inline string const & Match(unsigned char c);
	///
	void InsertException(KmodException & exclist, char c,
			     string const & data, bool = false,
			     tex_accent = TEX_NOACCENT);
	///
	void FreeException(KmodException & exclist);

	///
	string name_;
#if 0
	///
	string keymap_[256];
#else
	std::map<int, string> keymap_;
#endif
#if 0
	///
	kmod_list_decl * kmod_list_[TEX_MAX_ACCENT+1];
#else
	///
	//KmodInfo * kmod_list_[TEX_MAX_ACCENT+1];
	std::map<int, KmodInfo> kmod_list_;
#endif
};


///
string const & Trans::Match(unsigned char c)
{
#if 0
	return keymap_[c];
#else
	std::map<int, string>::iterator it = keymap_.find(c);
	if (it != keymap_.end()) {
		return it->second;
	}
	static string dummy;
	return dummy;
#endif
}

#endif 
