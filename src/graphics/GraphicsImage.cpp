/**
 * \file GraphicsImage.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsImage.h"
#include "GraphicsParams.h"

#include "support/debug.h"


namespace lyx {
namespace graphics {

// This is to be connected to a function that will return a new
// instance of a viable derived class.
boost::function<Image *()> Image::newImage;

} // namespace graphics
} // namespace lyx
