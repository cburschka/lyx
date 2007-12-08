// -*- C++ -*-
/**
 * \file ProgressInterface.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_SUPPORT_PROGRESSINTERFACE_H
#define LYX_SUPPORT_PROGRESSINTERFACE_H

class QString;

namespace lyx {
namespace support {


class ProgressInterface
{
public:
	virtual ~ProgressInterface() {}

	virtual void appendMessage(QString const &) = 0;
	virtual void clearMessages() = 0;

protected:
	ProgressInterface() {}
};


} // namespace support
} // namespace lyx

#endif // LYX_SUPPORT_PROGRESSINTERFACE_H

