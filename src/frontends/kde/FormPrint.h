/* FormPrint.h
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

#ifndef FORMPRINT_H
#define FORMPRINT_H

#include "DialogBase.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class PrintDialog;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

class FormPrint : public DialogBase, public noncopyable {
public: 
	/**@name Constructors and Destructors */
	//@{
	///
	FormPrint(LyXView *, Dialogs *);
	/// 
	~FormPrint();
	//@}

	/// start print
	void print();
	/// close
	void close();
 
private: 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update(bool = false);

	/// Real GUI implementation.
	PrintDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
};

#endif
