/**
 * \file WorkAreaFactory.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
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
