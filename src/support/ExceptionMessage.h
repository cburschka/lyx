// -*- C++ -*-
/**
 * \file ExceptionMessage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 *
 * A store of the paths to the various different directoies used
 * by LyX. These paths differ markedly from one OS to another,
 * following the local Windows, MacOS X or Posix conventions.
 */
#ifndef LYX_MESSAGE_H
#define LYX_MESSAGE_H

#include "support/docstring.h"

#include <exception>

namespace lyx {
namespace support {


enum ExceptionType {
	ErrorException,
	BufferException,
	WarningException
};


class ExceptionMessage: public std::exception {
public:
	ExceptionMessage(ExceptionType type, docstring const & title,
		docstring const & details)
	: type_(type), title_(title), details_(details),
	  message_(to_utf8(title_ + docstring::value_type('\n') + details_)) {}

	virtual const char * what() const throw() { return message_.c_str(); }
	virtual ~ExceptionMessage() throw() {}

	ExceptionType type_;
	docstring title_;
	docstring details_;
	// Needed because we may not return a temporary in what().
	std::string message_;
};

} // namespace support
} // namespace lyx

#endif // LYX_MESSAGE_H
