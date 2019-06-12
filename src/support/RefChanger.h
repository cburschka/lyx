// -*- C++ -*-
/**
 * \file RefChanger.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_REFCHANGER_H
#define LYX_REFCHANGER_H

#include "support/Changer.h"


namespace lyx {

/// A RefChanger records the current value of \param ref, allowing to
/// temporarily assign new values to it.  The original value is restored
/// automatically when the object is destroyed, unless it is disabled.
///
/// RefChanger is movable, and doing so prolongs the duration of the temporary
/// assignment. This allows classes to supply their own changer methods.
///
/// Naturally, be careful not to extend the life of a RefChanger beyond that of
/// the reference it modifies. The RefChanger can be disabled by calling
/// ->keep() or ->revert(). Once disabled, the reference is never accessed
/// again.
template<typename X>
class RevertibleRef : public Revertible {
public:
	RevertibleRef(X & ref) : ref(ref), old(ref), enabled(true) {}
	//
	~RevertibleRef() { revert(); }
	//
	void revert() {	if (enabled) { enabled = false; ref = old; } }
	//
	void keep() { enabled = false; }
	//
	X & ref;
	X const old;
private:
	bool enabled;
};


template <typename X> using RefChanger = unique_ptr<RevertibleRef<X>>;


/// Saves the value of \param ref in a movable object
template <typename X> RefChanger<X> make_save(X & ref)
{
	return make_unique<RevertibleRef<X>>(ref);
}

/// Temporarily assign value val to reference ref.
/// To apply the change conditionnally, one can write:
///     Changer dummy = (cond) ? make_change(a, b) : Changer();
template <typename X>
RefChanger<X> make_change(X & ref, X const val)
{
	auto rc = make_save(ref);
	ref = val;
	return rc;
}


} // namespace lyx


#endif //LYX_REFCHANGER_H
