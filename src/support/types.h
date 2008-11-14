// -*- C++ -*-
/**
 * \file types.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Provide a set of typedefs for commonly used things like sizes and
 * indices wile trying to stay compatible with types used
 * by the standard containers.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_TYPES_H
#define LYX_TYPES_H

#include <cstddef>

namespace lyx {

	/// a type for positions used in paragraphs
	// needs to be signed for a while to hold the special value -1 that is
	// used there
	typedef ptrdiff_t  pos_type;

	/// a type for paragraph offsets
	// FIXME: should be unsigned as well.
	// however, simply changing it breaks a downward loop somewhere...
	typedef ptrdiff_t  pit_type;

	/// a type for the nesting depth of a paragraph
	typedef size_t     depth_type;

// set this to '0' if you want to have really safe types
#if 1

	/// a type for sizes
	typedef size_t     size_type;

#else

	// These structs wrap simple things to make them distinguishible
	// to the compiler.
	// It's a shame that different typedefs are not "really" different

	struct size_type {
		///
		typedef size_t  base_type;
		///
		size_type(base_type t) { data_ = t; }
		///
		operator base_type() const { return data_; }
		///
		private:
		base_type data_;
	};

#endif

	///
	enum word_location {
		// the word around the cursor, only if the cursor is
		//not at a boundary
		WHOLE_WORD_STRICT,
		// the word around the cursor
		WHOLE_WORD,
		/// the word begining from the cursor position
		PARTIAL_WORD,
		/// the word around the cursor or before the cursor
		PREVIOUS_WORD,
		/// the next word (not yet used)
		NEXT_WORD
	};

	///
	enum PageSides {
		///
		OneSide,
		///
		TwoSides
	};

} // namespace lyx

#endif // LYX_TYPES_H
