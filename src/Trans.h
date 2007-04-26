// -*- C++ -*-
/**
 * \file Trans.h
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

#include "KmodInfo.h"

#include <map>


namespace lyx {


class Lexer;

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
	docstring const process(char_type, TransManager &);
	///
	bool isAccentDefined(tex_accent, KmodInfo &) const;

private:
	///
	void addDeadkey(tex_accent, docstring const &);
	///
	void freeKeymap();
	///
	int load(Lexer &);
	///
	docstring const & match(char_type c);
	///
	void insertException(KmodException & exclist, char_type c,
	                     docstring const & data, bool = false,
			     tex_accent = TEX_NOACCENT);
	///
	void freeException(KmodException & exclist);

	///
	std::string name_;
	///
	std::map<char_type, docstring> keymap_;
	///
	std::map<tex_accent, KmodInfo> kmod_list_;
};


///
inline
docstring const & Trans::match(char_type c)
{
	std::map<char_type, docstring>::iterator it = keymap_.find(c);
	if (it != keymap_.end()) {
		return it->second;
	}
	static docstring dummy;
	return dummy;
}


} // namespace lyx

#endif // TRANS_H
