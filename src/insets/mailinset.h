// -*- C++ -*-
/**
 * \file mailinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef MAILINSET_H
#define MAILINSET_H


#include "LString.h"


class InsetBase;


class MailInset {
public:
	///
	virtual ~MailInset() {};
	///
	void showDialog() const;
	///
	void updateDialog() const;
	///
	void hideDialog() const;
	///
	virtual string const inset2string() const = 0;

protected:
	///
	virtual InsetBase & inset() const = 0;
	///
	virtual string const & name() const = 0;
};


#endif // MAILINSET_H
