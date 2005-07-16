// -*- C++ -*-
/**
 * \file paper.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 *
 * A trivial header file to hold paper-related enums. It should later
 * expand to contain many paper-related horrors access.
 */

#ifndef PAPER_H
#define PAPER_H

///
enum PAPER_PACKAGES {
	///
	PACKAGE_NONE,
	///
	PACKAGE_A4,
	///
	PACKAGE_A4WIDE,
	///
	PACKAGE_WIDEMARGINSA4
};

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
	PAPER_A3,
	///
	PAPER_A4,
	///
	PAPER_A5,
	///
	PAPER_B3,
	///
	PAPER_B4,
	///
	PAPER_B5
};

///
enum PAPER_ORIENTATION {
	///
	ORIENTATION_PORTRAIT,
	///
	ORIENTATION_LANDSCAPE
};
#endif
