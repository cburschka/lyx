// -*- C++ -*-
/**
 * \file TocBackend.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Abdelrazak Younes
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOC_BACKEND_H
#define TOC_BACKEND_H

#include "DocIterator.h"
#include "FuncRequest.h"
#include "OutputEnums.h"

#include "support/shared_ptr.h"
#include "support/strfwd.h"

#include <map>
#include <vector>
#include <stack>
#include <string>


namespace lyx {

class Buffer;


/* FIXME: toc types are currently identified by strings. It cannot be converted
 * into an enum because of the user-configurable indexing categories and
 * the user-definable float types provided by layout files.
 *
 * I leave this for documentation purposes for the moment.
 *
enum TocType {
	TABLE_OF_CONTENTS,//"tableofcontents"
	CHILD,//"child"
	GRAPHICS,//"graphics"
	NOTE,//"note"
	BRANCH,//"branch"
	CHANGE,//"change"
	LABEL,//"label"
	CITATION,//"citation"
	EQUATION,//"equation"
	FOOTNOTE,//"footnote"
	MARGINAL_NOTE,//"marginalnote"
	INDEX,//"index", "index:<user-str>" (from interface)
	NOMENCL,//"nomencl"
	LISTING,//"listings"
	FLOAT,//"figure", "table", "algorithm", user-defined (from layout?)
	MATH_MACRO,//"math-macro"
	EXTERNAL,//"external"
	SENSELESS,//"senseless"
	TOC_TYPE_COUNT
}
 */

///
/**
*/
class TocItem
{
	friend class Toc;
	friend class TocBackend;
	friend class TocBuilder;

public:
	/// Default constructor for STL containers.
	TocItem() : dit_(0), depth_(0), output_(false) {}
	///
	TocItem(DocIterator const & dit,
		int depth,
		docstring const & s,
		bool output_active,
		docstring const & t = docstring(),
		FuncRequest action = FuncRequest(LFUN_UNKNOWN_ACTION)
		);
	///
	~TocItem() {}
	///
	int id() const;
	///
	int depth() const { return depth_; }
	///
	docstring const & str() const { return str_; }
	///
	void str(docstring const & s) { str_ = s; }
	///
	docstring const & tooltip() const;
	/// String for display, e.g. it has a mark if output is inactive
	docstring const asString() const;
	///
	DocIterator const & dit() const { return dit_; }
	///
	bool isOutput() const { return output_; }
	///
	void setAction(FuncRequest a) { action_ = a; }
	/// custom action, or the default one (paragraph-goto) if not customised
	FuncRequest action() const;

protected:
	/// Current position of item.
	DocIterator dit_;

private:
	/// nesting depth
	int depth_;
	/// Full item string
	docstring str_;
	/// The tooltip string
	docstring tooltip_;
	/// Is this item in a note, inactive branch, etc?
	bool output_;
	/// Custom action
	FuncRequest action_;
};


///
class Toc : public std::vector<TocItem>
{
public:
	typedef std::vector<TocItem>::const_iterator const_iterator;
	typedef std::vector<TocItem>::iterator iterator;
	const_iterator item(DocIterator const & dit) const;
	/// Look for a TocItem given its depth and string.
	/// \return The first matching item.
	/// \retval end() if no item was found.
	iterator item(int depth, docstring const & str);
};

typedef Toc::const_iterator TocIterator;


/// Caption-enabled TOC builders
class TocBuilder
{
public:
	TocBuilder(shared_ptr<Toc> const toc);
	/// When entering a float
	void pushItem(DocIterator const & dit, docstring const & s,
				  bool output_active, bool is_captioned = false);
	/// When encountering a caption
	void captionItem(DocIterator const & dit, docstring const & s,
					 bool output_active);
	/// When exiting a float
	void pop();
private:
	TocBuilder(){}
	///
	struct frame {
		Toc::size_type pos;
		bool is_captioned;
	};
	///
	shared_ptr<Toc> const toc_;
	///
	std::stack<frame> stack_;
};


/// The ToC list.
/// A class and no typedef because we want to forward declare it.
class TocList : public std::map<std::string, shared_ptr<Toc> >
{
private:
	// this can create null pointers
	using std::map<std::string, shared_ptr<Toc> >::operator[];
};


///
class TocBuilderStore
{
public:
	TocBuilderStore() {};
	///
	shared_ptr<TocBuilder> get(std::string const & type, shared_ptr<Toc> toc);
	///
	void clear() { map_.clear(); };
private:
	typedef std::map<std::string, shared_ptr<TocBuilder> > map_t;
	map_t map_;
};


///
/**
*/
class TocBackend
{
public:
	///
	TocBackend(Buffer const * buffer) : buffer_(buffer) {}
	///
	void setBuffer(Buffer const * buffer) { buffer_ = buffer; }
	///
	void update(bool output_active, UpdateType utype);
	/// \return true if the item was updated.
	bool updateItem(DocIterator const & pit);
	///
	TocList const & tocs() const { return tocs_; }
	/// never null
	shared_ptr<Toc const> toc(std::string const & type) const;
	shared_ptr<Toc> toc(std::string const & type);
	/// nevel null
	shared_ptr<TocBuilder> builder(std::string const & type);
	/// Return the first Toc Item before the cursor
	TocIterator item(
		std::string const & type, ///< Type of Toc.
		DocIterator const & dit ///< The cursor location in the document.
	) const;

	///
	void writePlaintextTocList(std::string const & type,
	        odocstringstream & os, size_t max_length) const;
	///
	docstring outlinerName(std::string const & type) const;

private:
	///
	TocList tocs_;
	///
	TocBuilderStore builders_;
	///
	Buffer const * buffer_;
}; // TocBackend


} // namespace lyx

#endif // TOC_BACKEND_H
