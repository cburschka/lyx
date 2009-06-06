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
	InsetFoot(Buffer const &);
private:
	///
	InsetCode lyxCode() const { return FOOT_CODE; }
	///
	docstring name() const { return from_ascii("Foot"); }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring editMessage() const;
	/// Update the counters of this inset and of its contents
	void updateLabels(ParIterator const &);
	///
	void addToToc(DocIterator const &);
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	Inset * clone() const { return new InsetFoot(*this); }
	///
	docstring custom_label_;
};


} // namespace lyx

#endif
