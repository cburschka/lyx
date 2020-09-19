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

#include "support/strfwd.h"

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
	Floating() : usesfloatpkg_(false), ispredefined_(false),
	    allowswide_(true), allowssideways_(true) {}
	///
	Floating(std::string const & type, std::string const & placement,
		 std::string const & ext, std::string const & within,
		 std::string const & style, std::string const & name,
		 std::string const & listName, std::string const & listCmd,
		 std::string const & refPrefix, std::string const & allowedplacement,
		 std::string const & htmlType, std::string const & htmlClass,
		 docstring const & htmlStyle,
		 std::string const & docbookAttr, std::string const & docbookTagType,
		 std::string const & required, bool usesfloat, bool isprefined,
		 bool allowswide, bool allowssideways);
	///
	std::string const & floattype() const { return floattype_; }
	///
	std::string docbookFloatType() const;
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
	/// allowed placement options
	std::string const & allowedPlacement() const { return allowedplacement_; }
	///
	bool usesFloatPkg() const { return usesfloatpkg_; }
	/// allowed placement options
	std::string const & required() const { return required_; }
	///
	bool isPredefined() const { return ispredefined_; }
	///
	bool allowsWide() const { return allowswide_; }
	///
	bool allowsSideways() const { return allowssideways_; }
	/// style information, for preamble
	docstring const & htmlStyle() const { return html_style_; }
	/// class, for css, defaults to "float-" + type()
	std::string const & htmlAttrib() const;
	/// tag type, defaults to "div"
	std::string const & htmlTag() const;
	///
	std::string docbookTag(bool hasTitle = false) const;
	///
	std::string const & docbookAttr() const;
	///
	std::string const & docbookTagType() const;
	///
	std::string const & docbookCaption() const;
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
	std::string allowedplacement_;
	///
	std::string required_;
	///
	bool usesfloatpkg_;
	///
	bool ispredefined_;
	///
	bool allowswide_;
	///
	bool allowssideways_;
	///
	mutable std::string html_tag_;
	///
	mutable std::string html_attrib_;
	///
	mutable std::string defaultcssclass_;
	///
	docstring html_style_;
	// There is no way to override the DocBook tag based on the layouts: half of it is determined by whether the float
	// has a title or not, an information that is not available in the layouts.
	/// attribute (mostly, role)
	mutable std::string docbook_caption_;
	/// caption tag (mostly, either caption or title)
	std::string docbook_attr_;
	/// DocBook tag type (block, paragraph, inline)
	mutable std::string docbook_tag_type_;
};


} // namespace lyx

#endif
