// -*- C++ -*-
/**
 * \file Floating.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FLOATING_H
#define FLOATING_H

#include <string>


namespace lyx {

/** This is a "float layout" object. It contains the parameters for how to
 *  handle the different kinds of floats, default ones and user created ones.
 *  Objects of this class is stored in a container in FloatList. The different
 *  InsetFloat(s) have a pointer/reference through the name of the Floating
 *  so that it knows how the different floats should be handled.
 */
class Floating {
public:
	///
	Floating();
	///
	Floating(std::string const & type, std::string const & placement,
		 std::string const & ext, std::string const & within,
		 std::string const & style, std::string const & name,
		 std::string const & listName, std::string const & htmlType,
		 std::string const & htmlClass, std::string const & htmlStyle,
		 bool builtin = false);
	///
	std::string const & type() const;
	///
	std::string const & placement() const;
	///
	std::string const & ext() const;
	///
	std::string const & within() const;
	///
	std::string const & style() const;
	///
	std::string const & name() const;
	///
	std::string const & listName() const;
	/// style information, for preamble
	std::string const & htmlStyle() const;
	/// class, for css
	std::string const & htmlClass() const;
	/// tag type
	std::string const & htmlType() const;
	///
	bool builtin() const;
private:
	///
	std::string type_;
	///
	std::string placement_;
	///
	std::string ext_;
	///
	std::string within_;
	///
	std::string style_;
	///
	std::string name_;
	///
	std::string listName_;
	/// HTML Element type, usually div
	std::string htmlType_;
	/// class attribute, e.g., float-table, for CSS
	std::string htmlClass_;
	/// CSS information for this element
	std::string htmlStyle_;
	///
	bool builtin_;
};


} // namespace lyx

#endif
