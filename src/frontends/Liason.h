// -*- C++ -*-
/* Liason.h
 * Temporary namespace to hold the various frontend functions until XTL and
 * the compilers of the world are ready for something more elaborate.
 * This is basically the Communicator class from the lyx cvs module all
 * over again.
 *
 * Eventually, we will switch back to the XTL+LyXFunc combination that
 * worked so nicely on a very small number of compilers and systems (when
 * most systems can support those required features).
 *
 * Author: Allan Rae <rae@lyx.org>
 * This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 2000
 *           Allan Rae
 * ======================================================
 */

#ifndef LIASON_H
#define LIASON_H

class PrinterParams;
class Buffer;

/** Temporary namespace to hold the various frontend functions
 * until XTL and the compilers of the world are ready for something more
 * elaborate. This is basically the Communicator class from the lyx cvs module
 * all over again.
 *
 * Eventually, we will switch back to the XTL+LyXFunc combination that
 * worked so nicely on a very small number of compilers and systems.
 * See the "dialogbase" branch of lyx-devel cvs module for xtl implementation.
 */
#ifdef CXX_WORKING_NAMESPACES
namespace Liason
{
#endif
/**@name Global support functions */
//@{
/// get global printer parameters
PrinterParams getPrinterParams(Buffer *);
/// print the current buffer
bool printBuffer(Buffer *, PrinterParams const &);
//@}

#ifdef CXX_WORKING_NAMESPACES
}
#endif

#endif
