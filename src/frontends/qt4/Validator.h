// -*- C++ -*-
/**
 * \file Validator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 *
 * Validators are used to decide upon the legality of some input action.
 * For example, a "line edit" widget might be used to input a "glue length".
 * The correct syntax for such a length is "2em + 0.5em". The LengthValidator
 * below will report whether the input text conforms to this syntax.
 *
 * This information is used in LyX primarily to give the user some
 * feedback on the validity of the input data using the "checked_widget"
 * concept. For example, if the data is invalid then the label of
 * a "line edit" widget is changed in colour and the dialog's "Ok"
 * and "Apply" buttons are disabled. See checked_widgets.[Ch] for
 * further details.
 */

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "Length.h"
#include "Dialog.h" // KernelDocType

#include <QValidator>

class QWidget;
class QLineEdit;


namespace lyx {

class LyXRC;

namespace frontend {

/** A class to ascertain whether the data passed to the @c validate()
 *  member function can be interpretted as a GlueLength.
 */
class LengthValidator : public QValidator
{
	Q_OBJECT
public:
	/// Define a validator for widget @c parent.
	LengthValidator(QWidget * parent);

	/** @returns QValidator::Acceptable if @c data is a GlueLength.
	 *  If not, returns QValidator::Intermediate.
	 */
	QValidator::State validate(QString & data, int &) const;

	/** @name Bottom
	 *  Set and retrieve the minimum allowed Length value.
	 */
	//@{
	void setBottom(Length const &);
	void setBottom(GlueLength const &);
	Length bottom() const { return b_; }
	void setUnsigned(bool const u) { unsigned_ = u; }
	//@}

private:
	Length b_;
	GlueLength g_;
	bool no_bottom_;
	bool glue_length_;
	bool unsigned_;
};


/// @returns a new @c LengthValidator that does not accept negative lengths.
LengthValidator * unsignedLengthValidator(QLineEdit *);


/** @returns a new @c LengthValidator that does not accept negative lengths.
 *  but glue lengths.
 */
LengthValidator * unsignedGlueLengthValidator(QLineEdit *);


/** A class to ascertain whether the data passed to the @c validate()
 *  member function can be interpretted as a GlueLength or is @param autotext.
 */
class LengthAutoValidator : public LengthValidator
{
	Q_OBJECT
public:
	/// Define a validator for widget @c parent.
	LengthAutoValidator(QWidget * parent, QString const & autotext);

	/** @returns QValidator::Acceptable if @c data is a GlueLength
		* or is "auto". If not, returns QValidator::Intermediate.
	 */
	QValidator::State validate(QString & data, int &) const;

private:
	QString autotext_;
};

/// @returns a new @c LengthAutoValidator that does not accept negative lengths.
LengthAutoValidator * unsignedLengthAutoValidator(QLineEdit *, QString const & autotext);


/**
 * A class to determine whether the passed is a double
 * or is @param autotext.
 *
 */
class DoubleAutoValidator : public QDoubleValidator
{
	Q_OBJECT
public:
	DoubleAutoValidator(QWidget * parent, QString const & autotext);
	DoubleAutoValidator(double bottom, double top, int decimals,
		QObject * parent);
	QValidator::State validate(QString & input, int & pos) const;

private:
	QString autotext_;
};


// A class to ascertain that no newline characters are passed.
class NoNewLineValidator : public QValidator
{
	Q_OBJECT
public:
	// Define a validator.
	NoNewLineValidator(QWidget *);
	// Remove newline characters from input.
	QValidator::State validate(QString &, int &) const;
};


/** A class to ascertain whether the data passed to the @c validate()
 *  member function is a valid file path.
 *  The test is active only when the path is to be stored in a LaTeX
 *  file, LaTeX being quite picky about legal names.
 */
class PathValidator : public QValidator
{
	Q_OBJECT
public:
	/** Define a validator for widget @c parent.
	 *  If @c acceptable_if_empty is @c true then an empty path
	 *  is regarded as acceptable.
	 */
	PathValidator(bool acceptable_if_empty, QWidget * parent);

	/** @returns QValidator::Acceptable if @c data is a valid path.
	 *  If not, returns QValidator::Intermediate.
	 */
	QValidator::State validate(QString &, int &) const;

	/** Define what checks that @c validate() will perform.
	 *  @param doc_type checks are activated only for @c LATEX docs.
	 *  @param lyxrc contains a @c tex_allows_spaces member that
	 *  is used to define what is legal.
	 */
	void setChecker(KernelDocType const & doc_type, LyXRC const & lyxrc);

private:
	bool acceptable_if_empty_;
	bool latex_doc_;
	bool tex_allows_spaces_;
};


/// @returns the PathValidator attached to the widget, or 0.
PathValidator * getPathValidator(QLineEdit *);

} // namespace frontend
} // namespace lyx

# endif // NOT VALIDATOR_H
