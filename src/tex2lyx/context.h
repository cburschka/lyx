// -*- C++ -*-
#ifndef CONTEXT_H
#define CONTEXT_H

#include "lyxtextclass.h"

// A helper struct
struct Context {
	Context(bool need_layout_,
		LyXTextClass const & textclass_,
		LyXLayout_ptr layout_ = LyXLayout_ptr(),
		LyXLayout_ptr parent_layout_= LyXLayout_ptr());

	// Output a \begin_layout is requested
	void check_layout(std::ostream & os);

	// Output a \end_layout if needed
	void check_end_layout(std::ostream & os);

	// Output a \begin_deeper if needed
	void check_deeper(std::ostream & os);

	// Output a \end_deeper if needed
	void check_end_deeper(std::ostream & os);
	
	// dump content on stream (for debugging purpose), with
	// description \c desc.
	void dump(std::ostream &, std::string const & desc = "context") const;

	// Do we need to output some \begin_layout command before the
	// next characters?
	bool need_layout;
	// Do we need to output some \end_layout command
	bool need_end_layout;
	// We may need to add something after this \begin_layout command
	std::string extra_stuff;
	// If there has been an \begin_deeper, we'll need a matching
	// \end_deeper
	bool need_end_deeper;

	// The textclass of the document. Could actually be a global variable
	LyXTextClass const & textclass;
	// The layout of the current paragraph
	LyXLayout_ptr layout;
	// The layout of the outer paragraph (for environment layouts)
	LyXLayout_ptr parent_layout;
};
	

#endif
