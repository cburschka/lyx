/**
 * \file GraphicsTypes.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "graphics/GraphicsTypes.h"

#include <string>

using namespace std;

namespace lyx {
namespace graphics {

namespace {

/// The translator between the Display enum and corresponding lyx string.
Translator<DisplayType, string> const initTranslator()
{
	Translator<DisplayType, string> translator(DefaultDisplay, "default");

	// Fill the display translator
	translator.addPair(MonochromeDisplay, "monochrome");
	translator.addPair(GrayscaleDisplay, "grayscale");
	translator.addPair(ColorDisplay, "color");
	translator.addPair(NoDisplay, "none");

	return translator;
}

} // namespace anon

Translator<DisplayType, string> const & displayTranslator()
{
	static Translator<DisplayType, string> const translator =
		initTranslator();
	return translator;
}

} // namespace graphics
} // namespace lyx
