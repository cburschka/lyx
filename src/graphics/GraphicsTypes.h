// -*- C++ -*-
/**
 * \file GraphicsTypes.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * All that header files outside the graphics subdirectory should need to
 * access.
 */

#ifndef GRAPHICSTYPES_H
#define GRAPHICSTYPES_H

#include "support/Translator.h"
#include "support/strfwd.h"


namespace lyx {
namespace graphics {

/// The status of the loading process
enum ImageStatus {
	/** The data is in the cache, but no request to display it
	 *  has been received.
	 */
	WaitingToLoad,
	/// The image is in a loadable format and is being loaded.
	Loading,
	/// The image is being converted to a loadable format.
	Converting,
	/// The image has been loaded into memory.
	Loaded,
	/// The image is in memory and is being scaled, rotated, etc.
	ScalingEtc,
	/// All finished. Can display the image.
	Ready,
	///
	ErrorNoFile,
	///
	ErrorConverting,
	///
	ErrorLoading,
	///
	ErrorGeneratingPixmap,
	/// The data is not in the cache at all!
	ErrorUnknown
};

} // namespace graphics
} // namespace lyx

#endif // GRAPHICSTYPES_H
