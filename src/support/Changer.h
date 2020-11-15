// -*- C++ -*-
/**
 * \file Changer.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_CHANGER_H
#define LYX_CHANGER_H

#include "support/unique_ptr.h"


namespace lyx {

struct Revertible {
	virtual ~Revertible() = default;
};

using Changer = unique_ptr<Revertible>;


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
	~RevertibleRef() override { revert(); }
	//
	void revert() { if (enabled) { enabled = false; ref = old; } }
	//
	void keep() { enabled = false; }
	//
	X & ref;
	X const old;
private:
	bool enabled;
};


template <typename X>
using RefChanger = unique_ptr<RevertibleRef<X>>;


/// Saves the value of \param ref in a movable object
template <typename X>
inline RefChanger<X> make_save(X & ref)
{
	return make_unique<RevertibleRef<X>>(ref);
}

inline Changer noChange()
{
	return Changer();
}

/// Temporarily assign value val to reference ref.
/// To apply the change conditionally, one can write:
///     Changer dummy = (cond) ? changeVar(ref, val) : noChange();
template <typename X>
inline RefChanger<X> changeVar(X & ref, X const val)
{
	auto rc = make_save(ref);
	ref = val;
	return rc;
}

} // namespace lyx


#endif //LYX_CHANGER_H
