// -*- C++ -*-
/**
 * \file lyx_gui.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef LYX_GUI_H
#define LYX_GUI_H

#include "LColor.h"
#include "LString.h"
#include "FuncStatus.h"

#include <vector>

class Dialogs;
class LyXFont;
class LyXComm;
class FuncRequest;

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
void start(string const & batch, std::vector<string> const & files);

/**
 * quit running LyX
 */
void exit();


/**
 * return the status flag for a given action. This can be used to tell
 * that a given lfun is not implemented by a frontend
 */
FuncStatus getStatus(FuncRequest const & ev);

/** Eg, passing LColor::black returns "000000",
 *      passing LColor::white returns "ffffff".
 */
string const hexname(LColor::color col);

/**
 * update an altered GUI color
 */
void update_color(LColor::color col);

/**
 * update the font cache
 */
void update_fonts();

/**
 * is the given font available ?
 */
bool font_available(LyXFont const & font);

/**
 * add a callback for I/O read notification
 */
void set_read_callback(int fd, LyXComm * comm);

/**
 * remove a I/O read callback
 * @param fd file descriptor
 */
void remove_read_callback(int fd);

/**
 * tells whether a special metrics hack is needed by mathed. Currently
 * only Qt/Mac requires it. The name has been chosen to be horrible
 * enough to get someone to fix the fonts :)
 */
bool needs_ugly_metrics_hack();

} // namespace lyx_gui

#endif // LYX_GUI_H
