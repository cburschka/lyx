/* FormRef.h
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

#ifndef FORMREF_H
#define FORMREF_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/insetcommand.h"

class Dialogs;
class LyXView;
class FormRefDialog;

class FormRef : public DialogBase {
public: 
	/**@name Constructors and Destructors */
	//@{
	///
	FormRef(LyXView *, Dialogs *);
	/// 
	~FormRef();
	//@}

	/// double-click a ref
	void select(const char *);
	/// highlight a ref
	void highlight(const char *);
	/// set sort
	void set_sort(bool);
	/// goto a ref (or back)
	void goto_ref(); 
	/// update dialog
	void update(); 
	/// update just the refs
	void do_ref_update();
	/// Apply changes
	void apply();
	/// close the connections
	void close();
 
private: 
	enum Type {
		REF, PAGEREF, VREF, VPAGEREF, PRETTYREF
	}; 

	enum GotoType {
		GOTOREF, GOTOBACK
	};
 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
 
	/// create a Reference inset
	void createRef(string const &);
	/// edit a Reference inset
	void showRef(InsetCommand * const);
 
	/// update the keys list
	void updateRefs(void);
 
	/// Real GUI implementation.
	FormRefDialog * dialog_;

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
	/// is the inset we are reading from a readonly buffer ?
	bool readonly;
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
	/// Inset hide connection.
	Connection ih_;

	/// to sort or not to sort
	bool sort;
 
	/// where to go
	GotoType gotowhere;
 
	/// current type
	Type type;
 
	/// available references
	std::vector< string > refs;
};

#endif
