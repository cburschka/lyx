/**
 * \file FormIndex.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */
 
#ifndef FORMINDEX_H
#define FORMINDEX_H

#include "KFormBase.h"

class ControlIndex;
class IndexDialog;

class FormIndex : public KFormBase<ControlIndex, IndexDialog> {
public: 
	FormIndex(ControlIndex & c);

private: 
	/// apply changes
	virtual void apply();
	/// build the dialog
	virtual void build();
	/// update the dialog
	virtual void update();
};

#endif // FORMINDEX_H
