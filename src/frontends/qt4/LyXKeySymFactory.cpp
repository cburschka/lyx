/**
 * \file qt4/LyXKeySymFactory.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger & Jürgen
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/LyXKeySymFactory.h"

#include "QLyXKeySym.h"


namespace lyx {

namespace LyXKeySymFactory {

LyXKeySym * create()
{
	return new QLyXKeySym;
}

} // namespace LyXKeySymFactory


} // namespace lyx
