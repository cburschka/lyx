// -*- C++ -*-
/**
 * \file DocumentClassPtr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DOCUMENT_CLASS_PTR_H
#define DOCUMENT_CLASS_PTR_H

#include "support/shared_ptr.h"

namespace lyx {
class DocumentClass;

typedef shared_ptr<DocumentClass> DocumentClassPtr;
typedef shared_ptr<DocumentClass const> DocumentClassConstPtr;
}

#endif // DISPATCH_RESULT_H
