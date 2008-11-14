// -*- C++ -*-
/**
 * \file Variables.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef VARIABLES_H
#define VARIABLES_H

#include <map>


namespace lyx {


///
class Variables {
public:
	///
	void set(std::string const &, std::string const &);
	///
	std::string const get(std::string const &) const;
	///
	bool isSet(std::string const & var) const;
	///
	std::string const expand(std::string const &) const;
private:
	///
	typedef std::map<std::string, std::string> Vars;
	///
	Vars vars_;
};


} // namespace lyx

#endif
