// -*- C++ -*-
/**
 * \file QGraphics.h
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

#include "QDialogView.h"
#include <vector>

namespace lyx {
namespace frontend {

class ControlGraphics;
class QGraphicsDialog;

///
class QGraphics
	: public QController<ControlGraphics, QView<QGraphicsDialog> >
{
public:
	///
	friend class QGraphicsDialog;
	///
	QGraphics(Dialog &);
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
