/**
 * \file FormUrl.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMURL_H
#define FORMURL_H

#include "KFormBase.h" 

class ControlUrl;
class UrlDialog;

class FormUrl : public KFormBase<ControlUrl, UrlDialog> {
public: 
	FormUrl(ControlUrl & c);

private: 
	/// apply dialog
	virtual void apply();
	/// build dialog
	virtual void build();
	/// update dialog
	virtual void update();
};

#endif // FORMURL_H
