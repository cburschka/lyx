// -*- C++ -*-
/**
 * \file GuiImplementation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_H
#define GUI_H

#include "frontends/Gui.h"

#include <QObject>

#include <map>

namespace lyx {
namespace frontend {

class GuiViewBase;
class LyXView;

/**
 * The GuiImplementation class is the interface to all Qt4 components.
 */
class GuiImplementation : public QObject, public Gui
{
	Q_OBJECT

public:
	GuiImplementation();
	virtual ~GuiImplementation() {}


	virtual LyXView& createRegisteredView();
	virtual bool closeAllViews();
	virtual bool unregisterView(int id);

	virtual LyXView& view(int id) const;

private:

	/// Multiple views container.
	/**
	* Warning: This must not be a smart pointer as the destruction of the
	* object is handled by Qt when the view is closed
	* \sa Qt::WA_DeleteOnClose attribute.
	*/
	std::map<int, GuiViewBase *> views_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_H
