/**
 * \file BCView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BCView.h"
#include "ButtonController.h"

namespace lyx {
namespace frontend {

ButtonPolicy const & BCView::bp() const { return parent.policy(); }
ButtonPolicy & BCView::bp() { return parent.policy(); }


} // namespace frontend
} // namespace lyx
