// -*- C++ -*-
/**
 *  \file PreviewLoader.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 *
 *  grfx::PreviewLoader collects latex snippets together. Then, on a
 *  startLoading() call, these are dumped to file and processed, converting
 *  each snippet to a separate bitmap image file. Once a bitmap file is ready
 *  to be loaded back into LyX, the PreviewLoader emits a signal to inform
 *  the initiating process.
 */

#ifndef PREVIEWLOADER_H
#define PREVIEWLOADER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/signals/signal1.hpp>

class Buffer;

namespace grfx {

class PreviewImage;

class PreviewLoader : boost::noncopyable {
public:
	/** We need buffer because we require the preamble to the
	 *  LaTeX file.
	 */
	PreviewLoader(Buffer const & buffer);
	///
	~PreviewLoader();

	/** Is there an image already associated with this snippet of LaTeX?
	 *  If so, returns a pointer to it, else returns 0.
	 */
	PreviewImage const * preview(string const & latex_snippet) const;

	///
	enum Status {
		///
		NotFound,
		///
		InQueue,
		///
		Processing,
		///
		Ready
	};

	/// How far have we got in loading the image?
	Status status(string const & latex_snippet) const;

	/// Add a snippet of LaTeX to the queue for processing.
	void add(string const & latex_snippet) const;

	/// Remove this snippet of LaTeX from the PreviewLoader.
	void remove(string const & latex_snippet) const;

	/** We have accumulated several latex snippets with status "InQueue".
	 *  Initiate their transformation into bitmap images.
	 */
	void startLoading() const;

	/** Connect and you'll be informed when the bitmap image file
	 *  has been created and is ready for loading through
	 *  grfx::PreviewImage::image().
	 */
	typedef boost::signal1<void, PreviewImage const &>::slot_type slot_type;
	///
	boost::signals::connection connect(slot_type const &) const;

	/** When PreviewImage has finished loading the image file into memory,
	 *  it tells the PreviewLoader to tell the outside world
	 */
	void emitSignal(PreviewImage const &) const;

	/// Which buffer owns this loader.
	Buffer const & buffer() const;

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	boost::scoped_ptr<Impl> const pimpl_;
};

} // namespace grfx

#endif // PREVIEWLOADER_H
