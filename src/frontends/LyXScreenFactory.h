/**
 * \file LyXScreenFactory.h
 * Read the file COPYING
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef LYXSCREENFACTORY_H
#define LYXSCREENFACTORY_H
 
class WorkArea;
class LyXScreen;

namespace LyXScreenFactory {
	/**
	 * Make a screen renderer. Used because we want to 
	 * generate a toolkit-specific instance.
	 */
	LyXScreen * create(WorkArea & owner);
}

#endif // LYXSCREEN_FACTORY_H
