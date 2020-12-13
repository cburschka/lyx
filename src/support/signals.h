// -*- C++ -*-
/**
 * \file signals.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_SIGNALS_H
#define LYX_SIGNALS_H

#include <nod.hpp>

#include <memory>

namespace lyx {

namespace signals2 = ::nod;

namespace support {

/// A small utility to use with signals2::slot_type::track_foreign when the
/// parent object is not handled by a shared_ptr, or to track the lifetime of an
/// object. Using Trackable to track lifetimes is less thread-safe than tracking
/// their parents directly with a shared_ptr as recommended by signals2, but it
/// makes it easier for transitioning old code. (Essentially because Trackable
/// will not prevent the deletion of the parent by a concurrent thread.)
class Trackable {
public:
	Trackable() : p_(std::make_shared<int>(0)) {}
	Trackable(Trackable const &) : Trackable() {}
	Trackable(Trackable &&) : Trackable() {}
	Trackable & operator=(Trackable const &) { return *this; }
	Trackable & operator=(Trackable &&) { return *this; }
	// This weak pointer lets you know if the parent object has been destroyed
	std::weak_ptr<void> p() const { return p_; }
private:
	std::shared_ptr<void> const p_;
};

} // namespace support

} // namespace lyx


#endif
