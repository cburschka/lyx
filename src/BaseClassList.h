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
class BaseClassIndex {
public:
	///
	typedef std::string base_type;
	///
	BaseClassIndex(base_type t) { data_ = t; }
	///
	operator base_type() const { return data_; }
	///
private:
	base_type data_;
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
	TextClass const & operator[](std::string const & classname) const;
	/// Read textclass list.  Returns false if this fails.
	bool read();
	/// Clears the textclass so as to force it to be reloaded
	void reset(BaseClassIndex const & tc);
	/// add a textclass from user local directory.
	/// \return the identifier for the loaded file, or else an
	/// empty string if no file was loaded.
	BaseClassIndex
		addTextClass(std::string const & textclass, std::string const & path);
	/// a list of the available classes
	std::vector<BaseClassIndex> classList() const;
	/// 
	static std::string const localPrefix;
private:
	///
	typedef std::map<std::string, TextClass> ClassMap;
	/// noncopyable
	BaseClassList(BaseClassList const &);
	/// nonassignable
	void operator=(BaseClassList const &);
	///
	mutable ClassMap classmap_; //FIXME
};

///
BaseClassIndex defaultBaseclass();


} // namespace lyx

#endif
