// -*- C++ -*-
/**
 *  \file Previews.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 *
 *  grfx::Previews is a singleton class that stores the grfx::PreviewLoader
 *  for each buffer requiring one.
 */

#ifndef PREVIEWS_H
#define PREVIEWS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

class Buffer;

namespace grfx {

class PreviewLoader;

class Previews : boost::noncopyable {
public:
	/// a wrapper for lyxrc.preview
	static bool activated();

	/// This is a singleton class. Get the instance.
	static Previews & get();

	/** Returns the PreviewLoader for this buffer.
	 *  Used by individual insets to update their own preview.
	 *  We assert that (buffer != 0) but do not pass a Buffer &
	 *  so that insets do not need to #include buffer.h
	 */
	PreviewLoader & loader(Buffer const * buffer);

	/** Called from the Buffer d-tor.
	 *  If (buffer == 0), does nothing.
	 */
	void removeLoader(Buffer const * buffer);

	/** For a particular buffer, initiate the generation of previews
	 *  for each and every snippetof LaTeX that's of interest with
	 *  a single forked process.
	 *  If (buffer == 0), does nothing.
	 */
	void generateBufferPreviews(Buffer const & buffer);

private:
	/** Make the c-tor, d-tor private so we can control how many objects
	 *  are instantiated.
	 */
	Previews();
	///
	~Previews();

	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	boost::scoped_ptr<Impl> const pimpl_;
};

} // namespace grfx

#endif // PREVIEWS_H
