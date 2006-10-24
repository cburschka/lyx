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

class LyXView;

namespace lyx {
namespace frontend {

class GuiWorkArea;
class GuiView;

/**
 * The GuiImplementation class is the interface to all Qt4 components.
 */
class GuiImplementation: public QObject, public Gui
{
	Q_OBJECT

public:
	GuiImplementation();
	virtual ~GuiImplementation() {}

	virtual int newView();
	virtual LyXView& view(int id);
	virtual int newWorkArea(unsigned int width, unsigned int height, int view_id);
	virtual WorkArea& workArea(int id);
	virtual bool closeAll();

public Q_SLOTS:
	///
	void unregisterView(GuiView * view);

private:
	///
	void buildViewIds();

	/// Multiple views container.
	/**
	* Warning: This must not be a smart pointer as the destruction of the
	* object is handled by Qt when the view is closed 
	* \sa Qt::WA_DeleteOnClose attribute.
	*/
	std::map<int, GuiView *> views_;

	/// Multiple workareas container.
	/**
	* Warning: This must not be a smart pointer as the destruction of the
	* object is handled by Qt when its parent view is closed.
	*/
	std::map<int, GuiWorkArea *> work_areas_;
	///
	size_t max_view_id_;
	///
	size_t max_wa_id_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_H
