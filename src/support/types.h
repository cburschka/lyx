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

#include <boost/cstdint.hpp>

#include <cstddef>

namespace lyx {

	/// The type used to hold characters in paragraphs
#ifdef USE_WCHAR_T
	// Prefer this if possible because GNU libstdc++ has usable
	// std::ctype<wchar_t> locale facets but not
	// std::ctype<boost::uint32_t>. gcc older than 3.4 is also missing
	// usable std::char_traits<boost::uint32_t>.
	typedef wchar_t char_type;
#else
	typedef boost::uint32_t char_type;
#endif

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

	/// a type used for numbering text classes
	typedef size_t     textclass_type;

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

	struct textclass_type {
		///
		typedef size_t   base_type;
		///
		textclass_type(base_type t) { data_ = t; }
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

} // namespace lyx

#endif // LYX_TYPES_H
