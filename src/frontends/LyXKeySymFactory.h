/**
 * \file LyXKeySymFactory.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Asger & Juergen
 */

#ifndef LYXKEYSYMFACTORY_H
#define LYXKEYSYMFACTORY_H

class LyXKeySym;

namespace LyXKeySymFactory {
	/**
	 * Make a LyXKeySym. Used because we want to 
	 * generate a toolkit-specific instance.
	 */
	LyXKeySym * create();
}

#endif // LYXKEYSYM_FACTORY_H
