// -*- C++ -*-
/**
 * \file formula.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FORMULA_H
#define INSET_FORMULA_H

#include "formulabase.h"
#include "math_atom.h"

#include <boost/scoped_ptr.hpp>


/// The main LyX math inset
class InsetFormula : public InsetFormulaBase {
public:
	///
	InsetFormula(bool chemistry = false);
	///
	explicit InsetFormula(BufferView *);
	///
	explicit InsetFormula(std::string const & data);
	///
	InsetFormula(InsetFormula const &);
	///
	~InsetFormula();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const &, LyXLex & lex);
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const &, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const &, std::ostream &) const;
	///
	int docbook(Buffer const &, std::ostream &, bool mixcont) const;

	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetOld::Code lyxCode() const;
	///
	bool insetAllowed(InsetOld::Code code) const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &,
			  std::vector<std::string> & list) const;
	///
	MathAtom const & par() const { return par_; }
	///
	MathAtom & par() { return par_; }
	///
	void generatePreview(Buffer const &) const;
	///
	void addPreview(lyx::graphics::PreviewLoader &) const;
	///
	void mutate(std::string const & type);

private:
	/// Slot receiving a signal that the preview is ready to display.
	void statusChanged() const;
	/// available in AMS only?
	bool ams() const;

	/// contents
	MathAtom par_;
	/// x offset for drawing displayed formula
	mutable int offset_;

	/// Use the Pimpl idiom to hide the internals of the previewer.
	class PreviewImpl;
	friend class PreviewImpl;
	/// The pointer never changes although *preview_'s contents may.
	boost::scoped_ptr<PreviewImpl> const preview_;
};
#endif
