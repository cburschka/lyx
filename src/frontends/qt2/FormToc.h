/* FormToc.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 * Adapted for Qt2 frontend by Kalle Dalheimer, 
 *   kalle@klaralvdalens-datakonsult.se
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FORMTOC_H
#define FORMTOC_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/insetcommand.h"
#include "buffer.h"

class Dialogs;
class FormTocDialog;

class FormToc : public DialogBase {
public:
	/**@name Constructors and Destructors */
	//@{
	///
	FormToc(LyXView *, Dialogs *);
	///
	~FormToc();
	//@}

	// Build the dialog
	virtual void build();
	/// Update the dialog before showing it.
	virtual void update();
	/// close the connections
	virtual void close();

private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();

	/// create a Toc inset
	void createTOC(string const &);
	/// view a Toc inset
	void showTOC(InsetCommand * const);
	
	/// update the Toc
	void updateToc(int);

	/// Real GUI implementation.
	FormTocDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;

	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// pointer to the inset if any
	InsetCommand * inset_;
	/// insets params
	InsetCommandParams params;
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
	/// Inset hide connection.
	Connection ih_;

	/// the toc list
	std::vector <Buffer::TocItem> toclist;

	/// depth of list shown
	int depth;
};

#endif
