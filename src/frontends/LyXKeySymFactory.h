/**
 * \file LyXKeySymFactory.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger & Juergen
 *
 * Full author contact details are available in file CREDITS
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
