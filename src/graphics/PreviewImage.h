// -*- C++ -*-
/**
 * \file PreviewImage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PREVIEWIMAGE_H
#define PREVIEWIMAGE_H

#include "support/strfwd.h"

namespace lyx {

namespace support { class FileName; }

namespace graphics {

class PreviewLoader;
class Image;

class PreviewImage {
public:
	/** ascent = height * ascent_frac
	 *  descent = height * (1 - ascent_frac)
	 */
	PreviewImage(PreviewLoader & parent,
		     std::string const & latex_snippet,
		     support::FileName const & bitmap_file,
		     double ascent_frac);
	///
	~PreviewImage();

	///
	std::string const & snippet() const;
	///
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;

	/** If the image is not yet loaded (WaitingToLoad), then this method
	 *  triggers that.
	 */
	Image const * image() const;

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	Impl * const pimpl_;
};

} // namespace graphics
} // namespace lyx

#endif // PREVIEWIMAGE_H
