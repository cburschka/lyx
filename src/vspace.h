// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef VSPACE_H
#define VSPACE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxgluelength.h"

#include "LString.h"

class BufferParams;
class BufferView;


///
class VSpace {
public:
	/// The different kinds of spaces.
	enum vspace_kind {
		///
		NONE,
		///
		DEFSKIP,
		///
		SMALLSKIP,
		///
		MEDSKIP,
		///
		BIGSKIP,
		///
		VFILL,
		///
		LENGTH
	};
	/// Constructor
	VSpace();
	/// Constructor
	explicit VSpace(vspace_kind k);
	/// Constructor
	explicit VSpace(LyXLength const & l);
	/// Constructor
	explicit VSpace(LyXGlueLength const & l);

	/// Constructor for reading from a .lyx file
	explicit VSpace(string const & data);
	
	/// access functions
	vspace_kind kind() const;
	///
	LyXGlueLength length() const;

	// a flag that switches between \vspace and \vspace*
	bool keep() const;
	///
	void setKeep(bool val);
	///
	bool operator==(VSpace const &) const;

	// conversion
	/// how it goes into the LyX file
	string const asLyXCommand() const; 
	///
	string const asLatexCommand(BufferParams const & params) const;
	///
	int inPixels(BufferView * bv) const;
private:
	/// This VSpace kind
	vspace_kind kind_;
	///
	LyXGlueLength len_;
	///
	bool keep_;
};

#endif
