// -*- C++ -*-
/**
 *  \file GraphicsTypes.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 *  All that header files outside the graphics subdirectory should need to
 *  access. That just leaves insetgraphics.C to access GraphicsCache.h.
 *  It also makes life easier for files inside the graphics subdirectory!
 */

#ifndef GRAPHICSTYPES_H
#define GRAPHICSTYPES_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

namespace grfx {

	///
	class GImage;
	///
	typedef boost::shared_ptr<GImage> ImagePtr;

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
		/// The image is in memory and is being scaled, rotated, etc.
		ScalingEtc,
		/// All finished. Can display the image.
		Loaded,
		///
		ErrorNoFile,
		///
		ErrorConverting,
		///
		ErrorLoading,
		/// Fall back on the unmodified image?
		ErrorScalingEtc,
		/// The data is not in the cache at all!
		ErrorUnknown
	};
}

#endif // GRAPHICSTYPES_H
