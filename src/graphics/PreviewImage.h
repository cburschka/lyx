// -*- C++ -*-
/**
 *  \file PreviewImage.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef PREVIEWIMAGE_H
#define PREVIEWIMAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <boost/scoped_ptr.hpp>

namespace grfx {

class PreviewLoader;
class Image;

class PreviewImage {
public:
	/** ascent = height * ascent_frac
	 *  descent = height * (1 - ascent_frac)
	 */
	PreviewImage(PreviewLoader & parent,
		     string const & latex_snippet,
		     string const & bitmap_file,
		     double ascent_frac);
	///
	~PreviewImage();

	/// We are explicit about when we begin the loading process.
	void startLoading();

	///
	string const & snippet() const;
	///
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;
	///
	Image const * image() const;

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	boost::scoped_ptr<Impl> const pimpl_;
};

} // namespace grfx

#endif // PREVIEWIMAGE_H
