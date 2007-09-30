// -*- C++ -*-
/**
 * \file Previews.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * lyx::graphics::Previews is a singleton class that stores the
 * lyx::graphics::PreviewLoader for each buffer requiring one.
 */

#ifndef PREVIEWS_H
#define PREVIEWS_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace lyx {

class Buffer;
class LyXRC_PreviewStatus;

namespace graphics {

class PreviewLoader;

class Previews : boost::noncopyable {
public:
	/// a wrapper for lyxrc.preview
	static LyXRC_PreviewStatus status();

	/// This is a singleton class. Get the instance.
	static Previews & get();

	/** Returns the PreviewLoader for this buffer.
	 *  Used by individual insets to update their own preview.
	 */
	PreviewLoader & loader(Buffer const & buffer) const;

	/// Called from the Buffer d-tor.
	void removeLoader(Buffer const & buffer) const;

	/** For a particular buffer, initiate the generation of previews
	 *  for each and every snippet of LaTeX that's of interest with
	 *  a single forked process.
	 */
	void generateBufferPreviews(Buffer const & buffer) const;

private:
	/** Make the c-tor, d-tor private so we can control how many objects
	 *  are instantiated.
	 */
	Previews();
	~Previews();

	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	boost::scoped_ptr<Impl> const pimpl_;
};

} // namespace graphics
} // namespace lyx

#endif // PREVIEWS_H
