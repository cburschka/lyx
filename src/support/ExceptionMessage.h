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
 */
#ifndef LYX_MESSAGE_H
#define LYX_MESSAGE_H

#include "support/docstring.h"

#include <exception>

namespace lyx {
namespace support {

/// LyX support three types of custom exceptions. In order of
/// increasing seriousness, these are:
///
/// WarningException
///   Intended for unexpected situations that we do not expect
///   to compromise further operation. It has the effect of
///   aborting whatever operation in in process.
///
/// BufferException
///   Intended for situations that indicate some problem with a
///   Buffer or its related data structures. The Buffer will be
///   closed, in emergency style.
///
/// ErrorException
///   Intended for situations that indicate a global problem
///   with the program. It will lead to an emergency shutdown.

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

	const char * what() const noexcept override { return message_.c_str(); }
	virtual ~ExceptionMessage() noexcept {}

	ExceptionType type_;
	docstring title_;
	docstring details_;
	// Needed because we may not return a temporary in what().
	std::string message_;
};

} // namespace support
} // namespace lyx

#endif // LYX_MESSAGE_H
