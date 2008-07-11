// -*- C++ -*-
/**
 * \file LayoutFile.h
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

#include <boost/noncopyable.hpp>

#include <vector>


namespace lyx {

class Layout;

/// Reads the style files
extern bool LyXSetStyle();


/// Index into LayoutFileList. Basically a 'strong typedef'.
class LayoutFileIndex {
public:
	///
	typedef std::string base_type;
	///
	LayoutFileIndex(base_type t) { data_ = t; }
	///
	operator base_type() const { return data_; }
	///
	bool empty() const { return data_.empty(); }
private:
	base_type data_;
};

/// This class amounts to little more than a `strong typedef'.
/// 
/// A LayoutFile represents the layout information that is 
/// contained in a *.layout file.
/// 
/// No document- (that is, Buffer-) specific information should 
/// be placed in these objects. They are used as the basis for 
/// constructing DocumentClass objects, which are what represent
/// the layout information associated with a Buffer. (This is also 
/// a subclass of TextClass, implemented in TextClass.{h,cpp}.)
/// Buffer-specific information should therefore be placed in a
/// DocumentClass object.
/// 
class LayoutFile : public TextClass, boost::noncopyable {
public:
	/// check whether the TeX class is available
	bool isTeXClassAvailable() const { return texClassAvail_; }
private:
	/// Construct a layout with default values. Actual values loaded later.
	explicit LayoutFile(std::string const & filename,
			std::string const & className = std::string(),
			std::string const & description = std::string(),
			bool texClassAvail = false);
	/// The only class that should create a LayoutFile is
	/// LayoutFileList, which calls the private constructor.
	friend class LayoutFileList;
	/// can't create empty LayoutFile
	LayoutFile() {};
};


/// A list of base document classes (*.layout files).
/// This is a singleton class. The sole instance is accessed
/// via LayoutFileList::get()
class LayoutFileList {
public:
	///
	LayoutFileList() {}
	~LayoutFileList();
	/// \return The sole instance of this class.
	static LayoutFileList & get();
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

	/// add a default textclass with all standard layouts.
	LayoutFileIndex addDefaultClass(std::string const & textclass);

	/// add a textclass from user local directory.
	/// \return the identifier for the loaded file, or else an
	/// empty string if no file was loaded.
	LayoutFileIndex
		addLocalLayout(std::string const & textclass, std::string const & path);
	/// a list of the available classes
	std::vector<LayoutFileIndex> classList() const;
private:
	///
	typedef std::map<std::string, LayoutFile *> ClassMap;
	/// noncopyable
	LayoutFileList(LayoutFileList const &);
	/// nonassignable
	void operator=(LayoutFileList const &);
	///
	mutable ClassMap classmap_; //FIXME
};

///
LayoutFileIndex defaultBaseclass();


} // namespace lyx

#endif
