/**
 * \file sgml.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author José Matos
 * \author John Levon <levon@movementarian.org>
 */

#ifndef SGML_H
#define SGML_H
 
#include <config.h>

#include "LString.h"
 
#include <algorithm>
 
namespace sgml {

/**
 * Escape the given character if necessary
 * to an SGML entity. The bool return is true
 * if it was a whitespace character.
 */
std::pair<bool, string> escapeChar(char c);

}

#endif // SGML_H
