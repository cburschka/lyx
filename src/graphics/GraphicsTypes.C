// -*- C++ -*-
/**
 *  \file GraphicsTypes.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 */

#include "graphics/GraphicsTypes.h"

namespace grfx {

/// The translator between the Display enum and corresponding lyx string.
Translator< DisplayType, string > displayTranslator(DefaultDisplay, "default");

void setDisplayTranslator()
{
	/// This variable keeps a tab on whether the translator is set.
	static bool done = false;

	if (!done) {
		done = true;

		// Fill the display translator
		displayTranslator.addPair(DefaultDisplay, "default");
		displayTranslator.addPair(MonochromeDisplay, "monochrome");
		displayTranslator.addPair(GrayscaleDisplay, "grayscale");
		displayTranslator.addPair(ColorDisplay, "color");
		displayTranslator.addPair(NoDisplay, "none");
		
		// backward compatibility for old lyxrc.display_graphics
		displayTranslator.addPair(MonochromeDisplay, "mono");
		displayTranslator.addPair(GrayscaleDisplay, "gray");
		displayTranslator.addPair(NoDisplay, "no");
	}
}

} // namespace grfx
