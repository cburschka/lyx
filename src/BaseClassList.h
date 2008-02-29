// -*- C++ -*-
/**
 * \file BaseClassList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASECLASSLIST_H
#define BASECLASSLIST_H

#include "TextClass.h"

#include "support/strfwd.h"

#include <vector>


namespace lyx {

class Layout;

/// Reads the style files
extern bool LyXSetStyle();


/// Index into BaseClassList. Basically a 'strong typedef'.
class LayoutFileIndex {
public:
	///
	typedef std::string base_type;
	///
	LayoutFileIndex(base_type t) { data_ = t; }
	///
	operator base_type() const { return data_; }
	///
private:
	base_type data_;
};

/// This class amounts to little more than a `strong typedef'.
/// A LayoutFile represents the layout information that is 
/// contained in a *.layout file.
class LayoutFile : public TextClass {
public:
	/// This should never be used, but it has to be provided for
	/// std::map operator[] to work. Something like:
	///   mapthingy[stuff] = otherthing
	/// creates an empty object before doing the assignment.
	LayoutFile() {}
	/// check whether the TeX class is available
	bool isTeXClassAvailable() const { return texClassAvail_; }
private:
	/// Construct a layout with default values. Actual values loaded later.
	explicit LayoutFile(std::string const &,
		                 std::string const & = std::string(),
		                 std::string const & = std::string(),
		                 bool texClassAvail = false);
	/// The only class that should create a LayoutFile is
	/// BaseClassList, which calls the private constructor.
	friend class BaseClassList;
};


/// A list of base document classes (*.layout files).
/// This is a singleton class. The sole instance is accessed
/// via BaseClassList::get()
class BaseClassList {
public:
	///
	BaseClassList() {}
	/// \return The sole instance of this class.
	static BaseClassList & get();
	///
	bool empty() const { return classmap_.empty(); }
	///
	bool haveClass(std::string const & classname) const;
	///
	LayoutFile const & operator[](std::string const & classname) const;
	///
	LayoutFile & operator[](std::string const & classname);
	/// Read textclass list. Returns false if this fails.
	bool read();
	/// Clears the textclass so as to force it to be reloaded
	void reset(LayoutFileIndex const & tc);
	/// add a textclass from user local directory.
	/// \return the identifier for the loaded file, or else an
	/// empty string if no file was loaded.
	LayoutFileIndex
		addLayoutFile(std::string const & textclass, std::string const & path);
	/// a list of the available classes
	std::vector<LayoutFileIndex> classList() const;
	/// 
	static std::string const localPrefix;
private:
	///
	typedef std::map<std::string, LayoutFile> ClassMap;
	/// noncopyable
	BaseClassList(BaseClassList const &);
	/// nonassignable
	void operator=(BaseClassList const &);
	///
	mutable ClassMap classmap_; //FIXME
};

///
LayoutFileIndex defaultBaseclass();


} // namespace lyx

#endif
