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
 * graphics::Previews is a singleton class that stores the
 * graphics::PreviewLoader for each buffer requiring one.
 */

#ifndef PREVIEWS_H
#define PREVIEWS_H

namespace lyx {

class Buffer;
class LyXRC_PreviewStatus;

namespace graphics {

class PreviewLoader;

class Previews {
public:
	/// a wrapper for lyxrc.preview
	static LyXRC_PreviewStatus status();

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
	friend class LyX;
	/// noncopyable
	Previews(Previews const &);
	void operator=(Previews const &);

	/** Make the c-tor, d-tor private so we can control how many objects
	 *  are instantiated.
	 */
	Previews();
};

} // namespace graphics

/// This is a singleton class. Get the instance.
/// Implemented in LyX.cpp.
graphics::Previews * thePreviews();

} // namespace lyx

#endif // PREVIEWS_H
