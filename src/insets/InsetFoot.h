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
	InsetFoot(Buffer *);
private:
	///
	InsetCode lyxCode() const { return FOOT_CODE; }
	///
	docstring layoutName() const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// Update the counters of this inset and of its contents
	void updateBuffer(ParIterator const &, UpdateType);
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype) const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	Inset * clone() const { return new InsetFoot(*this); }
	///
	docstring custom_label_;
	///
	bool intitle_;
};


} // namespace lyx

#endif
