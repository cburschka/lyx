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
 enum PAPER_SIZE {
	 ///
	 PAPER_DEFAULT,
	 ///
	 PAPER_USLETTER,
	 ///
	 PAPER_LEGALPAPER,
	 ///
	 PAPER_EXECUTIVEPAPER,
	 ///
	 PAPER_A3PAPER,
	 ///
	 PAPER_A4PAPER,
	 ///
	 PAPER_A5PAPER,
	 ///
	 PAPER_B5PAPER
 };

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
enum VMARGIN_PAPER_TYPE {
	///
	VM_PAPER_DEFAULT,
	///
	VM_PAPER_CUSTOM,
	///
	VM_PAPER_USLETTER,
	///
	VM_PAPER_USLEGAL,
	///
	VM_PAPER_USEXECUTIVE,
	///
	VM_PAPER_A3,
	///
	VM_PAPER_A4,
	///
	VM_PAPER_A5,
	///
	VM_PAPER_B3,
	///
	VM_PAPER_B4,
	///
	VM_PAPER_B5
};

///
enum PAPER_ORIENTATION {
	///
	ORIENTATION_PORTRAIT,
	///
	ORIENTATION_LANDSCAPE
};
#endif
