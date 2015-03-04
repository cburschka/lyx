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
	Floating() : usesfloatpkg_(false), ispredefined_(false) {}
	///
	Floating(std::string const & type, std::string const & placement,
		 std::string const & ext, std::string const & within,
		 std::string const & style, std::string const & name,
		 std::string const & listName, std::string const & listCmd,
		 std::string const & refPrefix,
		 std::string const & htmlType, std::string const & htmlClass, 
		 std::string const & htmlStyle, bool usesfloat, bool isprefined);
	///
	std::string const & floattype() const { return floattype_; }
	///
	std::string const & placement() const { return placement_; }
	///
	std::string const & ext() const {return ext_; }
	///
	std::string const & within() const { return within_; }
	///
	std::string const & style() const { return style_; }
	///
	std::string const & name() const { return name_; }
	/// the title of a list of this kind of float
	std::string const & listName() const { return listname_; }
	/// the command used to generate that list. this has to be given
	/// if usesFloatPkg() is false, unless this float uses the same
	/// auxfile as another defined previously. this should not contain
	/// the leading "\".
	std::string const & listCommand() const { return listcommand_; }
	/// prefix to use for formatted references to such floats
	std::string const & refPrefix() const { return refprefix_; }
	///
	bool usesFloatPkg() const { return usesfloatpkg_; }
	///
	bool isPredefined() const { return ispredefined_; }
	/// style information, for preamble
	std::string const & htmlStyle() const { return html_style_; }
	/// class, for css, defaults to "float-" + type()
	std::string const & htmlAttrib() const;
	/// tag type, defaults to "div"
	std::string const & htmlTag() const;
private:
	///
	std::string defaultCSSClass() const;
	///
	std::string floattype_;
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
	std::string listname_;
	///
	std::string listcommand_;
	///
	std::string refprefix_;
	///
	bool usesfloatpkg_;
	///
	bool ispredefined_;
	/// 
	mutable std::string html_tag_;
	/// 
	mutable std::string html_attrib_;
	///
	mutable std::string defaultcssclass_;
	/// 
	std::string html_style_;
};


} // namespace lyx

#endif
