// -*- C++ -*-
/**
 * \file xforms/checkedwidgets.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CHECKEDWIDGETS_H
#define CHECKEDWIDGETS_H

#include "BCView.h"
#include "forms_fwd.h"

namespace lyx {
namespace frontend {

class CheckedLyXLength;
class CheckedGlueLength;
class CheckedPath;


//
CheckedLyXLength &
addCheckedLyXLength(BCView & bcview, FL_OBJECT * input, FL_OBJECT * label = 0);

CheckedGlueLength &
addCheckedGlueLength(BCView & bcview, FL_OBJECT * input, FL_OBJECT * label = 0);

CheckedPath &
addCheckedPath(BCView & bcview, bool acceptable_if_empty,
	       FL_OBJECT * input, FL_OBJECT * label = 0);


class CheckedLyXLength : public CheckedWidget {
public:
	/** The label widget's label will be turned red if input
	 *  does not make a valid LyXLength.
	 *  If label == 0, then the label of input will be used.
	 */
	CheckedLyXLength(FL_OBJECT * input, FL_OBJECT * label = 0);

private:
	///
	virtual bool check() const;

	///
	FL_OBJECT * input_;
	FL_OBJECT * label_;
};


class CheckedGlueLength : public CheckedWidget {
public:
	/** The label widget's label will be turned red if input
	 *  does not make a valid LyXGlueLength.
	 *  If label == 0, then the label of input will be used.
	 */
	CheckedGlueLength(FL_OBJECT * input, FL_OBJECT * label = 0);

private:
	///
	virtual bool check() const;

	///
	FL_OBJECT * input_;
	FL_OBJECT * label_;
};

} // namespace frontend
} // namespace lyx


// Forward declarations
class LyXRC;

namespace lyx {
namespace frontend {

class KernelDocType;


class CheckedPath : public CheckedWidget {
public:
	/** The label widget's label will be turned red if input
	 *  does not make a valid file path.
	 *  If label == 0, then the label of input will be used.
	 *  If @c acceptable_if_empty is @c true then an empty path
	 *  is regarded as acceptable.
	 */
	CheckedPath(bool acceptable_if_empty,
		    FL_OBJECT * input, FL_OBJECT * label = 0);

	/** Define now to perform the check.
	 *  @param doc_type checks are activated only for @c LATEX docs.
	 *  @param lyxrc contains a @c tex_allows_spaces member that
	 *  is used to define what is legal.
	 */
	void setChecker(lyx::frontend::KernelDocType const & doc_type,
			LyXRC const & lyxrc);

private:
	///
	virtual bool check() const;

	///
	FL_OBJECT * input_;
	FL_OBJECT * label_;

	bool acceptable_if_empty_;
	bool latex_doc_;
	bool tex_allows_spaces_;
};

} // namespace frontend
} // namespace lyx

#endif // CHECKEDWIDGETS_H
