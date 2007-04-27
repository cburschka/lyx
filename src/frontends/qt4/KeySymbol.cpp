/**
 * \file qt4/KeySymbolFactory.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger & Jürgen
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/KeySymbol.h"

#include "QKeySymbol.h"

namespace lyx {

KeySymbol * createKeySymbol()
{
	return new QKeySymbol;
}

} // namespace lyx
