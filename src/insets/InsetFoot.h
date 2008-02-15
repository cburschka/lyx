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
class InsetFoot : public InsetFootlike {
public:
	///
	InsetFoot(BufferParams const &);
	///
	InsetCode lyxCode() const { return FOOT_CODE; }
	///
	docstring name() const { return from_ascii("Foot"); }
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	///
	virtual docstring const editMessage() const;
	/// Update the counters of this inset and of its contents
	void updateLabels(Buffer const &, ParIterator const &);
	///
	void addToToc(Buffer const &, ParConstIterator const &) const;

protected:
	InsetFoot(InsetFoot const &);
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif
