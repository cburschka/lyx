// -*- C++ -*-
/**
 * \file lyx_gui.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_GUI_H
#define LYX_GUI_H


#include "FuncStatus.h"

#include <boost/function.hpp>

#include <string>
#include <vector>

class Dialogs;
class LColor_color;
class LyXFont;
class LyXComm;
class FuncRequest;
class LyXView;
namespace lyx {
struct RGBColor;
}

/// GUI interaction
namespace lyx_gui {

/// are we using the GUI at all
extern bool use_gui;

/**
 * set up GUI parameters. At this point lyxrc may
 * be used.
 */
void parse_lyxrc();

/**
 * Enter the main event loop (\sa LyX::exec2)
 */
int exec(int & argc, char * argv[]);

/**
 * Synchronise all pending events.
 */
void sync_events();

/**
 * return the status flag for a given action. This can be used to tell
 * that a given lfun is not implemented by a frontend
 */
FuncStatus getStatus(FuncRequest const & ev);

/**
 * Given col, fills r, g, b in the range 0-255.
 * The function returns true if successful.
 * It returns false on failure and sets r, g, b to 0.
 */
bool getRGBColor(LColor_color col, lyx::RGBColor & rgbcol);

/** Eg, passing LColor::black returns "000000",
 *      passing LColor::white returns "ffffff".
 */
std::string const hexname(LColor_color col);

/**
 * update an altered GUI color
 */
void update_color(LColor_color col);

/**
 * add a callback for socket read notification
 * @param fd socket descriptor (file/socket/etc)
 */
void register_socket_callback(int fd, boost::function<void()> func);

/**
 * remove a I/O read callback
 * @param fd socket descriptor (file/socket/etc)
 */
void unregister_socket_callback(int fd);

} // namespace lyx_gui

#endif // LYX_GUI_H
