/* FormTabularCreate.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FORMTABULARCREATE_H
#define FORMTABULARCREATE_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView; 
class TabularCreateDlgImpl;

class FormTabularCreate : public DialogBase {
public: 
	/**@name Constructors and Destructors */
	//@{
	///
	FormTabularCreate(LyXView *, Dialogs *);
	/// 
	~FormTabularCreate();
	//@}

	/// create the table 
	void apply(int rows, int cols);
	/// close the connections
	void close();
 
private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
 
	/// Real GUI implementation.
	TabularCreateDlgImpl * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	
	/// Hide connection.
	Connection h_;
};

#endif
