// -*- C++ -*-
/**
 * \file InsetFoot.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETFOOT_H
#define INSETFOOT_H

#include "InsetFootlike.h"


namespace lyx {

/** The footnote inset

*/
class InsetFoot : public InsetFootlike
{
public:
	///
	explicit InsetFoot(Buffer *);
private:
	///
	InsetCode lyxCode() const { return FOOT_CODE; }
	///
	docstring layoutName() const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	void docbook(XMLStream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures & features) const;
	/// Update the counters of this inset and of its contents
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false);
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	Inset * clone() const { return new InsetFoot(*this); }
	///
	bool inheritFont() const { return true; }
	///
	bool allowMultiPar() const { return true; }
	///
	docstring custom_label_;
	///
	bool intitle_;
	///
	bool infloattable_;
};


} // namespace lyx

#endif
