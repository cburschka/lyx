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
	///
	void AddDeadkey(tex_accent, string const &);
	///
	void FreeKeymap();
	///
	int Load(LyXLex &);
	///
	string const & Match(unsigned char c);
	///
	void InsertException(KmodException & exclist, char c,
			     string const & data, bool = false,
			     tex_accent = TEX_NOACCENT);
	///
	void FreeException(KmodException & exclist);

	///
	string name_;
	///
	std::map<int, string> keymap_;
	///
	std::map<int, KmodInfo> kmod_list_;
};


///
inline
string const & Trans::Match(unsigned char c)
{
	std::map<int, string>::iterator it = keymap_.find(c);
	if (it != keymap_.end()) {
		return it->second;
	}
	static string dummy;
	return dummy;
}

#endif
