// -*- C++ -*-
/**
 * \file GuiGraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QGRAPHICS_H
#define QGRAPHICS_H

#include "GuiDialogView.h"
#include "GuiGraphicsDialog.h"

#include <vector>

namespace lyx {
namespace frontend {

class ControlGraphics;

///
class GuiGraphics
	: public QController<ControlGraphics, GuiView<GuiGraphicsDialog> >
{
public:
	///
	friend class GuiGraphicsDialog;
	///
	GuiGraphics(Dialog &);
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
	/// get bounding box from file
	void getBB();

	/// Store the LaTeX names for the rotation origins.
	std::vector<std::string> origin_ltx;

};

} // namespace frontend
} // namespace lyx

#endif // QGRAPHICS_H
