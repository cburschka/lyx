/**
 * \file FormRef.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMREF_H
#define FORMREF_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/insetcommand.h"

class Dialogs;
class LyXView;
class RefDialog;

class FormRef : public DialogBase, public noncopyable {
public: 
	FormRef(LyXView *, Dialogs *);

	~FormRef();

	/// double-click a ref
	void select(char const *);
	/// highlight a ref
	void highlight(char const *);
	/// set sort
	void set_sort(bool);
	/// goto a ref (or back)
	void goto_ref(); 
	/// update dialog
	void update(bool = false); 
	/// update just the refs
	void do_ref_update();
	/// Apply changes
	void apply();
	/// close the connections
	void close();
 
private: 
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
	RefDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/// dialogs object
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
 
	/// available references
	std::vector< string > refs;
};

#endif // FORMREF_H
