// -*- C++ -*-
/**
 * \file LAssert.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef LASSERT_H
#define LASSERT_H

namespace lyx {
namespace support {

#ifdef ENABLE_ASSERTIONS

/** Live assertion.
    This is a debug tool to ensure that the assertion holds. If it
    doesn't hold we run #emergencyCleanup()# and then #lyx::abort".
    @param assertion this should evaluate to true unless you want an abort.
*/
void Assert(bool assertion);
/// same as above, print message before aborting
void Assert(bool assertion, const char * message);

#else

/** Dummy assertion.
    When compiling without assertions we use this no-op function.
*/
inline
void Assert(bool /*assertion*/) {}
inline
void Assert(bool /*assertion*/, const char * /*message*/) {}

#endif /* ENABLE_ASSERTIONS */

} // namespace support
} // namespace lyx

#endif /* LASSERT_H */
