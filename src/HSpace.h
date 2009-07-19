// -*- C++ -*-
/**
 * \file HSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef HSPACE_H
#define HSPACE_H

#include "Length.h"


namespace lyx {


class BufferParams;
class BufferView;


/// A class representing latex horizontal spacing
class HSpace {
public:
	/// The different kinds of spaces.
	enum HSpaceKind {
		DEFAULT,
		LENGTH ///< user-defined length
	};

	///
	HSpace();
	///
	explicit HSpace(HSpaceKind k);
	///
	explicit HSpace(Length const & l);
	///
	explicit HSpace(GlueLength const & l);

	/// Constructor for reading from a .lyx file
	explicit HSpace(std::string const & data);

	/// return the type of vertical space
	HSpaceKind kind() const { return kind_; }
	/// return the length of this space
	GlueLength const & length() const { return len_; }

	///
	bool operator==(HSpace const &) const;

	// conversion

	/// how it goes into the LyX file
	std::string const asLyXCommand() const;
	/// the latex representation
	std::string const asLatexCommand() const;
	///
	std::string asHTMLLength() const;
	/// how it is seen in the LyX window
	docstring const asGUIName() const;
	/// the size of the space on-screen
	int inPixels(BufferView const & bv) const;

private:
	/// This HSpace kind
	HSpaceKind kind_;
	/// the specified length
	GlueLength len_;
};


} // namespace lyx

#endif // HSPACE_H
