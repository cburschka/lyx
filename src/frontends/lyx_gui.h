/**
 * \file lyx_gui.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef LYX_GUI_H
#define LYX_GUI_H
 
#include <config.h>
 
#include "LString.h"
 
#include <vector>
 
/// GUI interaction
namespace lyx_gui {
 
	/// parse command line and do basic initialisation
	void parse_init(int & argc, char * argv[]);
	 
	/**
	 * set up GUI parameters. At this point lyxrc may
	 * be used. 
	 */
	void parse_lyxrc();

	/**
	 * Start the main event loop, after executing the given
	 * batch commands, and loading the given documents
	 */
	void start(string const & batch, std::vector<std::string> files);

	/// initialise graphics
	void init_graphics();
};

#endif // LYX_GUI_H
