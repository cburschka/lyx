// -*- C++ -*-
/**
 * \file lengthvalidator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LENGTHVALIDATOR_H
#define LENGTHVALIDATOR_H

#include "lyxlength.h"
#include "lyxgluelength.h"
#include <qvalidator.h>

class QWidget;


class LengthValidator : public QValidator
{
	Q_OBJECT
public:
	LengthValidator(QWidget * parent, const char *name = 0);

	QValidator::State validate(QString &, int &) const;

	void setBottom(LyXLength const &);
	void setBottom(LyXGlueLength const &);
	LyXLength bottom() const { return b_; }

private:
#if defined(Q_DISABLE_COPY)
	LengthValidator( const LengthValidator & );
	LengthValidator& operator=( const LengthValidator & );
#endif

	LyXLength b_;
	LyXGlueLength g_;
	bool no_bottom_;
	bool glue_length_;
};

# endif // NOT LENGTHVALIDATOR_H
