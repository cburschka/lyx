/**
 * \file foreach.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 *
 */

#ifndef FOREACH_H
#define FOREACH_H

// Code stolen from Q_FOREACH, augmented to use a reference to the
// original container instead of a copy. Copies are cheap (if not
// mutated) for Qt's containers due to copy-on-write. The are less
// cheap for Standard containers, that's why the modification.
// Drawback is that we can't use temporary containers as they
// will be destroyed before the loop is finished. So always write
//  
//  Container const & container = functionReturningTemporaryOrReference()
//  foreach (ContainerItem const & item, container) {
//   ...
//  }
//
// to extend the lifetime of the reference.

#if defined(Q_CC_GNU) && !defined(Q_CC_INTEL)

/* make use of typeof-extension */
template <typename T>
class ForeachContainer {
public:
    inline ForeachContainer(const T & t) : c(t), brk(0), i(c.begin()), e(c.end()) { }
    const T & c;
    int brk;
    typename T::const_iterator i, e;
};

#define foreach(variable, container)                                  \
for (ForeachContainer<__typeof__(container)> _container_(container);  \
     !_container_.brk && _container_.i != _container_.e;              \
     __extension__  ({ ++_container_.brk; ++_container_.i; }))        \
    for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;}))

#else 

struct ForeachContainerBase {};

template <typename T>
class ForeachContainer : public ForeachContainerBase {
public:
    inline ForeachContainer(const T& t): c(t), brk(0), i(c.begin()), e(c.end()){};
    const T & c;
    mutable int brk;
    mutable typename T::const_iterator i, e;
    inline bool condition() const { return (!brk++ && i != e); }
};

template <typename T> inline T *foreachPointer(const T &) { return 0; }

template <typename T> inline ForeachContainer<T> foreachContainerNew(const T& t)
{ return ForeachContainer<T>(t); }

template <typename T>
inline const ForeachContainer<T> *foreachContainer(const ForeachContainerBase *base, const T *)
{ return static_cast<const ForeachContainer<T> *>(base); }

#define foreach(variable, container) \
	for (const ForeachContainerBase &_container_ = foreachContainerNew(container); \
			 foreachContainer(&_container_, true ? 0 : foreachPointer(container))->condition();       \
			 ++foreachContainer(&_container_, true ? 0 : foreachPointer(container))->i)               \
			for (variable = *foreachContainer(&_container_, true ? 0 : foreachPointer(container))->i; \
					 foreachContainer(&_container_, true ? 0 : foreachPointer(container))->brk;           \
					 --foreachContainer(&_container_, true ? 0 : foreachPointer(container))->brk)
#endif

#endif // FOREACH_H
