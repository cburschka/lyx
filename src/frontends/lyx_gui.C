/**
 * \file frontends/lyx_gui.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyx_gui.h"

#include "Application.h"

using std::string;

lyx::frontend::Application * theApp;


namespace lyx_gui {

bool use_gui = true;

}; // namespace lyx_gui
