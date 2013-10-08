// -*- C++ -*-
/**
 * \file mutex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 *
 * A collection of string helper functions that works with string.
 * Some of these would certainly benefit from a rewrite/optimization.
 */

#ifndef MUTEX_H
#define MUTEX_H


namespace lyx {


class Mutex
{
	/// noncopyable
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&);
public:
	Mutex();
	~Mutex();
	
	/// Scope based locking:
	/// Usage:
	/// >>> unlocked
	/// { 
	///     Mutex::Locker locker(a_Mutex_ptr);  
	///     >>> locked
	/// }
	/// >>> unlocked
	class Locker
	{
	public:
		Locker(Mutex*);
		~Locker();

	private:
		Locker();
		Locker(const Locker& rhs);
		Locker& operator=(const Locker& rhs);
		Mutex* mutex_;
	};

private:
	struct Private;
	Private* d;
};


} // namespace lyx

#endif
