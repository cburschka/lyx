// -*- C++ -*-
/**
 * \file trans.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TRANS_H
#define TRANS_H

#include "trans_decl.h"

#include <map>


class LyXLex;

class TransManager;

/**
  Trans: holds a .kmap file
  */
class Trans {
public:
	///
	Trans();
	///
	~Trans();

	///
	int Load(std::string const & language);
	///
	bool IsDefined() const;
	///
	std::string const & GetName() const;
	///
	std::string const process(char, TransManager &);
	///
	bool isAccentDefined(tex_accent, KmodInfo &) const;

private:
	///
	void AddDeadkey(tex_accent, std::string const &);
	///
	void FreeKeymap();
	///
	int Load(LyXLex &);
	///
	std::string const & Match(unsigned char c);
	///
	void InsertException(KmodException & exclist, char c,
			     std::string const & data, bool = false,
			     tex_accent = TEX_NOACCENT);
	///
	void FreeException(KmodException & exclist);

	///
	std::string name_;
	///
	std::map<int, std::string> keymap_;
	///
	std::map<int, KmodInfo> kmod_list_;
};


///
inline
std::string const & Trans::Match(unsigned char c)
{
	std::map<int, std::string>::iterator it = keymap_.find(c);
	if (it != keymap_.end()) {
		return it->second;
	}
	static std::string dummy;
	return dummy;
}

#endif // TRANS_H
