// -*- C++ -*-
/**
 *  \file GraphicsTypes.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Angus Leeming <leeming@lyx.org>
 *
 *  All that header files outside the graphics subdirectory should need to
 *  access.
 */

#ifndef GRAPHICSTYPES_H
#define GRAPHICSTYPES_H

#include "support/translator.h"

#ifdef __GNUG__
#pragma interface
#endif

namespace grfx {

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

	/// How is the image to be displayed on the LyX screen?
	enum DisplayType {
		///
		DefaultDisplay,
		///
		MonochromeDisplay,
		///
		GrayscaleDisplay,
		///
		ColorDisplay,
		///
		NoDisplay
	};
	
	/// The translator between the Display enum and corresponding lyx string.
	extern Translator< DisplayType, string > displayTranslator;

	void setDisplayTranslator();
	
} // namespace grfx

#endif // GRAPHICSTYPES_H
