/**
 * \file FormLog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMLOG_H
#define FORMLOG_H

#include "KFormBase.h"

class ControlLog;
class LogDialog;

class FormLog : public KFormBase<ControlLog, LogDialog> {
public: 
	FormLog(ControlLog & c); 

	/// update the dialog
	virtual void update();

	/// apply
	virtual void apply() {};

	/// build dialog
	virtual void build(); 
};

#endif // FORMLOG_H
