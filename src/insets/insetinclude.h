// -*- C++ -*-
/**
 * \file insetinclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_INCLUDE_H
#define INSET_INCLUDE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

#include <boost/signals/signal0.hpp>
#include <boost/scoped_ptr.hpp>

class Buffer;
struct LaTeXFeatures;

// Created by AAS 970521

/// for including tex/lyx files
class InsetInclude: public InsetButton, boost::noncopyable {
public:
	/// the type of inclusion
	enum Flags {
		INCLUDE = 0, //<
		VERB = 1, //<
		INPUT = 2, //<
		VERBAST = 3 //<
	};

	struct Params {
		Params(InsetCommandParams const & cp = InsetCommandParams("input"),
		       Flags f = INPUT,
		       string const & name = string())
			: cparams(cp), flag(f),
			  masterFilename_(name) {}
 
		InsetCommandParams cparams;
		Flags flag;
		string masterFilename_;

		///
		bool operator==(Params const &) const;
		///
		bool operator!=(Params const &) const;
	};

	///
	InsetInclude(Params const &);
	///
	InsetInclude(InsetCommandParams const &, Buffer const &);

	~InsetInclude();

	/// Override these InsetButton methods if Previewing
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;

	/// get the parameters
	Params const & params(void) const;
	/// set the parameters
	void set(Params const & params);

	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const;
	///
	Inset::Code lyxCode() const { return Inset::INCLUDE_CODE; }
	/// This returns the list of labels on the child buffer
	std::vector<string> const getLabelList() const;
	/// This returns the list of bibkeys on the child buffer
	std::vector< std::pair<string,string> > const getKeys() const;
	///
	void edit(BufferView *, int x, int y, mouse_button::state button);
	///
	void edit(BufferView * bv, bool front = true);
	///
	EDITABLE editable() const
	{
		return IS_EDITABLE;
	}
	/// With lyx3 we won't overload these 3 methods
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const *, LyXLex &);
	///
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
	///
	void validate(LaTeXFeatures &) const;

	/** Input inserts anything inside a paragraph.
	    Display can give some visual feedback
	*/
	bool display() const;

	/// return the filename stub of the included file
	string const getRelFileBaseName() const;

	/// return true if the file is or got loaded.
	bool loadIfNeeded() const;

	///
	void addPreview(grfx::PreviewLoader &) const;

	/// hide a dialog if about
	boost::signal0<void> hideDialog;
private:
	/// get the text displayed on the button
	string const getScreenLabel(Buffer const *) const;
	/// is this a verbatim include ?
	bool isVerbatim() const;
	/// get the filename of the master buffer
	string const getMasterFilename() const;
	/// get the included file name
	string const getFileName() const;

	/// the parameters
	Params params_;
	/// holds the entity name that defines the file location (SGML)
	string const include_label;

	/// Use the Pimpl idiom to hide the internals of the previewer.
	class PreviewImpl;
	friend class PreviewImpl;
	/// The pointer never changes although *preview_'s contents may.
	boost::scoped_ptr<PreviewImpl> const preview_;
};


inline bool InsetInclude::isVerbatim() const
{
	return params_.flag == VERB || params_.flag == VERBAST;
}

#endif // INSETINCLUDE_H
