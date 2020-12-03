// -*- C++ -*-
/**
 * \file DialogFactory.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Yuriy Skalko
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIALOG_FACTORY_H
#define DIALOG_FACTORY_H

#include <string>

namespace lyx {
namespace frontend {

class Dialog;
class GuiView;


Dialog * createDialog(GuiView & gv, std::string const & name);


} // namespace frontend
} // namespace lyx

#endif // DIALOG_FACTORY_H
