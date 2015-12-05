// -*- C++ -*-
/**
 * \file paper.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 *
 * A trivial header file to hold paper-related enums. It should later
 * expand to contain many paper-related horrors access.
 *
 * The supported paper sizes are those that are supported by the
 * LaTeX-package geometry.
 */

#ifndef PAPER_H
#define PAPER_H

namespace lyx {

///
enum PAPER_SIZE {
	///
	PAPER_DEFAULT,
	///
	PAPER_CUSTOM,
	///
	PAPER_USLETTER,
	///
	PAPER_USLEGAL,
	///
	PAPER_USEXECUTIVE,
	///
	PAPER_A0,
	///
	PAPER_A1,
	///
	PAPER_A2,
	///
	PAPER_A3,
	///
	PAPER_A4,
	///
	PAPER_A5,
	///
	PAPER_A6,
	///
	PAPER_B0,
	///
	PAPER_B1,
	///
	PAPER_B2,
	///
	PAPER_B3,
	///
	PAPER_B4,
	///
	PAPER_B5,
	///
	PAPER_B6,
	///
	PAPER_C0,
	///
	PAPER_C1,
	///
	PAPER_C2,
	///
	PAPER_C3,
	///
	PAPER_C4,
	///
	PAPER_C5,
	///
	PAPER_C6,
	///
	PAPER_JISB0,
	///
	PAPER_JISB1,
	///
	PAPER_JISB2,
	///
	PAPER_JISB3,
	///
	PAPER_JISB4,
	///
	PAPER_JISB5,
	///
	PAPER_JISB6
};

///
enum PAPER_ORIENTATION {
	///
	ORIENTATION_PORTRAIT,
	///
	ORIENTATION_LANDSCAPE
};

} // namespace lyx


#endif
