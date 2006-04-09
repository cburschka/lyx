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
	int load(std::string const & language);
	///
	bool isDefined() const;
	///
	std::string const & getName() const;
	///
	std::string const process(char, TransManager &);
	///
	bool isAccentDefined(tex_accent, KmodInfo &) const;

private:
	///
	void addDeadkey(tex_accent, std::string const &);
	///
	void freeKeymap();
	///
	int load(LyXLex &);
	///
	std::string const & match(unsigned char c);
	///
	void insertException(KmodException & exclist, char c,
			     std::string const & data, bool = false,
			     tex_accent = TEX_NOACCENT);
	///
	void freeException(KmodException & exclist);

	///
	std::string name_;
	///
	std::map<int, std::string> keymap_;
	///
	std::map<int, KmodInfo> kmod_list_;
};


///
inline
std::string const & Trans::match(unsigned char c)
{
	std::map<int, std::string>::iterator it = keymap_.find(c);
	if (it != keymap_.end()) {
		return it->second;
	}
	static std::string dummy;
	return dummy;
}

#endif // TRANS_H
