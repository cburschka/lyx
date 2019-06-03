// -*- C++ -*-
/**
 * \file MathRow.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ROW_H
#define MATH_ROW_H

#include "InsetMath.h"
#include "MathClass.h"

#include "ColorCode.h"

#include "support/docstring.h"

#include <vector>

namespace lyx {

class BufferView;
class Dimension;
class MetricsInfo;
class PainterInfo;

class InsetMath;
class MathData;

/*
 * While for editing purpose it is important that macros are counted
 * as a single element, this is not the case for display. To get the
 * spacing correct, it is necessary to dissolve all the macros that
 * can be, along with their arguments. Then one obtains a
 * representation of the MathData contents as a string of insets and
 * then spacing can be done properly.
 *
 * This is the purpose of the MathRow class.
 */
class MathRow
{
public:
	// What row elements can be
	enum Type {
		INSET, // this element is a plain inset
		BOX, // an empty box
		BEGIN, // an inset and/or a math array begins here
		END, // an inset and/or a math array ends here
		DUMMY // a dummy element (used before or after row)
	};

	// An elements, together with its spacing
	struct Element
	{
		///
		Element(MetricsInfo const & mi, Type t, MathClass mc = MC_UNKNOWN);

		/// Classifies the contents of the object
		Type type;
		/// the class of the element
		MathClass mclass;
		/// the spacing around the element
		int before, after;
		/// count wether the current mathdata is nested in macro(s)
		int macro_nesting;
		/// Marker type
		InsetMath::marker_type marker;

		/// When type is INSET
		/// the math inset (also for BEGIN and END)
		InsetMath const * inset;
		// Non empty when there is a completion to draw
		docstring compl_text;
		// the number of characters forming the unique part.
		size_t compl_unique_to;

		// type is BEGIN, END
		MathData const * ar;

		// type is BOX
		ColorCode color;
	};

	///
	MathRow(int asc = 0, int des = 0) : caret_ascent(asc), caret_descent(des) {};
	///
	typedef std::vector<Element> Elements;
	///
	typedef Elements::iterator iterator;
	///
	typedef Elements::const_iterator const_iterator;
	///
	iterator begin() { return elements_.begin(); }
	///
	iterator end() { return elements_.end(); }
	///
	const_iterator begin() const { return elements_.begin(); }
	///
	const_iterator end() const { return elements_.end(); }
	//
	void push_back(Element const & e) { elements_.push_back(e); }
	//
	Element & back() { return elements_.back(); }

	// create the math row by unwinding all macros in the MathData and
	// compute the spacings.
	MathRow(MetricsInfo & mi, MathData const * ar);

	//
	void metrics(MetricsInfo & mi, Dimension & dim);
	//
	void draw(PainterInfo & pi, int const x, int const y) const;

	/// superscript kerning
	int kerning(BufferView const *) const;

	/// useful when the caret visits this cell
	int caret_ascent, caret_descent;


private:
	// Index of the first inset element before position i
	int before(int i) const;
	// Index of the first inset element after position i
	int after(int i) const;

	///
	Elements elements_;
};

///
std::ostream & operator<<(std::ostream & os, MathRow::Element const & elt);

///
std::ostream & operator<<(std::ostream & os, MathRow const & mrow);


} // namespace lyx

#endif
