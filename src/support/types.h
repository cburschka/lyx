#ifndef LYX_TYPES_H
#define LYX_TYPES_H

// provide a set of typedefs for commonly used things like sizes and
// indices while trying to stay compatible with types used by the standard
// containers.


// this probably could be improved by using <cstddef>...
#include <vector>

namespace lyx
{
	/// a type for positions used in paragraphs
	// needs to be signed for a while to hold the special value -1 that is
	// used there...
	typedef std::vector<char>::difference_type   pos_type;


// set this to '0' if you want to have really safe types
#if 1

	/// a type for sizes
	typedef std::vector<char>::size_type         size_type;

	/// a type used for numbering layouts	within a text class
	// used to be LyXTextClass::size_type
	typedef std::vector<char>::size_type         layout_type;

	/// a type used for numbering text classes
	// used to be LyXTextClassList::size_type
	typedef std::vector<char>::size_type         textclass_type;

#else

	// These structs wrap simple things to make them distinguishible
	// to the compiler.
	// It's a shame that different typedefs are not "really" different

	struct size_type {
		///
		typedef std::vector<char>::size_type  base_type;
		///
		size_type(base_type t) { data_ = t; }
		///
		operator base_type() const { return data_; }
		///
		private:
		base_type data_;
	};
		

	struct layout_type {
		///
		typedef std::vector<char>::size_type  base_type;
		///
		layout_type(base_type t) { data_ = t; }
		///
		operator base_type() const { return data_; }
		///
		void operator++() { ++data_; }
		///
		private:
		base_type data_;
	};
		

	struct textclass_type {
		///
		typedef std::vector<char>::size_type  base_type;
		///
		textclass_type(base_type t) { data_ = t; }
		///
		operator base_type() const { return data_; }
		///
		private:
		base_type data_;
	};
		

#endif

}

#endif
