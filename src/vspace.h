// -*- C++ -*-
/**
 * \file vspace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef VSPACE_H
#define VSPACE_H

#include "lyxgluelength.h"


class BufferParams;
class BufferView;


/// A class representing latex vertical spacing
class VSpace {
public:
	/// The different kinds of spaces.
	enum vspace_kind {
		NONE, ///< no added vertical space
		DEFSKIP,
		SMALLSKIP,
		MEDSKIP,
		BIGSKIP,
		VFILL,
		LENGTH ///< user-defined length
	};

	VSpace();

	explicit VSpace(vspace_kind k);

	explicit VSpace(LyXLength const & l);

	explicit VSpace(LyXGlueLength const & l);

	/// Constructor for reading from a .lyx file
	explicit VSpace(std::string const & data);

	/// return the type of vertical space
	vspace_kind kind() const;
	/// return the length of this space
	LyXGlueLength const & length() const;

	// a flag that switches between \vspace and \vspace*
	bool keep() const;
	/// if set true, use \vspace* when type is not DEFSKIP
	void setKeep(bool val);
	///
	bool operator==(VSpace const &) const;

	// conversion

	/// how it goes into the LyX file
	std::string const asLyXCommand() const;
	/// the latex representation
	std::string const asLatexCommand(BufferParams const & params) const;
	/// the size of the space on-screen
	int inPixels(BufferView const & bv) const;

private:
	/// This VSpace kind
	vspace_kind kind_;
	/// the specified length
	LyXGlueLength len_;
	/// if true, use \vspace* type
	bool keep_;
};

#endif // VSPACE_H
