#ifndef LYX_TYPES_H
#define LYX_TYPES_H

// provide a set of typedefs for commonly used things like sizes and
// indices whil trying to stay compatible with typse used by the standard
// containers.


// this probably could be improved by using <cstddef>...
#include <vector>

namespace lyx
{
	/// a type for sizes
	typedef std::vector<char>::size_type         size_type;

	/// a type for positions used in paragraphs
	// needs to be signed for a while to hold the special value -1 that is
	// used there...
	typedef std::vector<char>::difference_type   pos_type;

	/// a type used for numbering layouts	
	typedef std::vector<char>::size_type         layout_type;

}

#endif
