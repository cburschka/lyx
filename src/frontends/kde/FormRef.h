/**
 * \file FormRef.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMREF_H
#define FORMREF_H

#include "KFormBase.h"

class ControlRef;
class RefDialog;

class FormRef : public KFormBase<ControlRef, RefDialog> {
public: 
	FormRef(ControlRef & c);

	/// double-click a ref
	void select(char const *);
	/// highlight a ref
	void highlight(char const *);
	/// set sort
	void set_sort(bool);
	/// goto a ref (or back)
	void goto_ref(); 
	/// update just the refs
	void do_ref_update();
 
private: 
	enum GotoType {
		GOTOREF, GOTOBACK
	};

	/// apply changes 
	virtual void apply(); 
	/// build dialog
	virtual void build();
	/// update dialog
	virtual void update();

	/// update the keys list
	void updateRefs(void);
 
	/// to sort or not to sort
	bool sort_;
 
	/// where to go
	GotoType gotowhere_;
 
	/// available references
	std::vector<string> refs_;
};

#endif // FORMREF_H
