// -*- C++ -*-

/** 
 *  \file formula.h
 *
 *  This file is part of LyX, the document processor.
 *  Licence details can be found in the file COPYING.
 *
 *  \author Alejandro Aguilar Sierra
 *  \author André Pönitz
 *
 *  Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FORMULA_H
#define INSET_FORMULA_H

#ifdef __GNUG__
#pragma interface
#endif

#include "formulabase.h"
#include "math_atom.h"

#include <boost/scoped_ptr.hpp>

/// The main LyX math inset 
class InsetFormula : public InsetFormulaBase {
public:
	///
	InsetFormula();
	///
	explicit InsetFormula(BufferView *);
	///
	explicit InsetFormula(const string & data);
	///
	InsetFormula(InsetFormula const &);
	///
	~InsetFormula();
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;

	///
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const *, LyXLex & lex);
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;

	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset::Code lyxCode() const;
	///
	bool insetAllowed(Inset::Code code) const;
	///
	std::vector<string> const getLabelList() const;
	///
	MathAtom const & par() const { return par_; }
	///
	MathAtom & par() { return par_; }
	///
	void generatePreview() const;
	///
	void addPreview(grfx::PreviewLoader &) const;
	///
	//void mutate(string const & type);

private:
	/// available in AMS only?
	bool ams() const;

	/// contents
	MathAtom par_;

	/// Use the Pimpl idiom to hide the internals of the previewer.
	class PreviewImpl;
	friend class PreviewImpl;
	/// The pointer never changes although *preview_'s contents may.
	boost::scoped_ptr<PreviewImpl> const preview_;
};
#endif
