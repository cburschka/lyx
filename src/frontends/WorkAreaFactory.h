/**
 * \file WorkAreaFactory.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef WORKAREAFACTORY_H
#define WORKAREAFACTORY_H
 
class WorkArea;
 
namespace WorkAreaFactory {
	/**
	 * Make a work area. Used because we want to generate
	 * a toolkit-specific instance.
	 */
	WorkArea * create(int x, int y, int w, int h);
}

#endif // WORKAREA_FACTORY_H
