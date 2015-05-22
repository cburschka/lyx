// -*- C++ -*-
/**
 * \file FloatList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FLOATLIST_H
#define FLOATLIST_H

#include "Floating.h"

#include <map>
#include <string>


namespace lyx {


///
class FloatList {
public:
	///
	typedef std::map<std::string, Floating> List;
	///
	typedef List::const_iterator const_iterator;
	///
	FloatList();
	///
	const_iterator begin() const;
	///
	const_iterator end() const;
	///
	void newFloat(Floating const & fl);
	///
	std::string const defaultPlacement(std::string const & t) const;
	///
	std::string const allowedPlacement(std::string const & t) const;
	///
	bool typeExist(std::string const & t) const;
	///
	Floating const & getType(std::string const & t) const;
	///
	void erase(std::string const & t);
	///
	const_iterator operator[](std::string const & t) const;
private:
	///
	List list;
};


} // namespace lyx

#endif
