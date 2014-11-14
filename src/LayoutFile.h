// -*- C++ -*-
/**
 * \file LayoutFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Richard Heck (typedefs and such)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASECLASSLIST_H
#define BASECLASSLIST_H

#include "LayoutModuleList.h"
#include "TextClass.h"

#include "support/strfwd.h"

#include <string>
#include <vector>


namespace lyx {

class Layout;

/// Index into LayoutFileList. Basically a 'strong typedef'.
class LayoutFileIndex {
public:
	///
	typedef std::string base_type;
	///
	LayoutFileIndex(base_type const & t) : data_(t) { }
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
class LayoutFile : public TextClass {
public:
	/// check whether the TeX class is available
	bool isTeXClassAvailable() const { return tex_class_avail_; }
	///
	LayoutModuleList const & defaultModules() const 
			{ return default_modules_; }
	///
 	LayoutModuleList const & providedModules() const 
 			{ return provided_modules_; }
	///
 	LayoutModuleList const & excludedModules() const 
 			{ return excluded_modules_; }
private:
	/// noncopyable
	LayoutFile(LayoutFile const &);
	void operator=(LayoutFile const &);
	/// Construct a layout with default values. Actual values loaded later.
	explicit LayoutFile(std::string const & filename,
			std::string const & className = std::string(),
			std::string const & description = std::string(),
			std::string const & prerequisites = std::string(),
			std::string const & category = std::string(),
			bool texclassavail = false);
	/// The only class that should create a LayoutFile is
	/// LayoutFileList, which calls the private constructor.
	friend class LayoutFileList;
	/// can't create empty LayoutFile
	LayoutFile() {}
};


/// A list of base document classes (*.layout files).
/// This is a singleton class. The sole instance is accessed
/// via LayoutFileList::get()
class LayoutFileList {
public:
	///
	~LayoutFileList();
	/// \return The sole instance of this class.
	static LayoutFileList & get();
	///
	bool empty() const { return classmap_.empty(); }
	///
	bool haveClass(std::string const & classname) const;
	/// Note that this will assert if we don't have classname, so
	/// check via haveClass() first.
	LayoutFile const & operator[](std::string const & classname) const;
	/// Note that this will assert if we don't have classname, so
	/// check via haveClass() first.
	LayoutFile & operator[](std::string const & classname);
	/// Read textclass list. Returns false if this fails.
	bool read();
	/// Clears the textclass so as to force it to be reloaded
	void reset(LayoutFileIndex const & tc);

	/// Add a default textclass with all standard layouts.
	/// Note that this will over-write any information we may have
	/// gotten from textclass.lst about this class.
	LayoutFileIndex addEmptyClass(std::string const & textclass);

	/// add a textclass from user local directory.
	/// \return the identifier for the loaded file, or else an
	/// empty string if no file was loaded.
	LayoutFileIndex
		addLocalLayout(std::string const & textclass, std::string const & path);
	/// a list of the available classes
	std::vector<LayoutFileIndex> classList() const;

	///
	bool load(std::string const & name, std::string const & buf_path);

private:
	///
	LayoutFileList() {}
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
