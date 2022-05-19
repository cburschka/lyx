// -*- C++ -*-
/* \file pmprof.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

/**
 * ==== HOW TO USE THIS TRIVIAL PROFILER:
 *
 * * at the beginning of the interesting block, just add:
 *   PROFILE_THIS_BLOCK(some_identifier)
 *
 *   A trailing semicolon can be added at your discretion.
 *
 * * when the program ends, statistics will be sent to standard error, like:
 *
 *   #pmprof# some_identifier: 6.51usec, count=7120, total=46.33msec
 *
 * * It is also possible to profile caching schemes. All it takes is an additional
 *   PROFILE_CACHE_MISS(some_identifier)
 *   in the place that takes care of cache misses. Then the output at the end will change to
 *
 *   #pmprof# some_identifier: 6.51usec, count=7120, total=46.33msec
 *      hit: 96%, 4.36usec, count=6849, total=29.89msec
 *     miss: 3%, 60.65usec, count=271, total=16.43msec
 *
 * * if DISABLE_PMPROF is defined before including pmprof.h, the
 *   profiler is replaced by empty macros. This is useful for quickly
 *   checking the overhead.
 *
 * ==== ABOUT PROFILING SCOPE:
 *
 * The code measured by the profiler corresponds to the lifetime of a
 * local variable declared by the PROFILE_THIS_BLOCK macro.
 *
 * Some examples of profiling scope: In the snippets below, c1, c2...
 * designate code chunks, and the identifiers of profiling blocks are
 * chosen to reflect what they count.
 *
 * {
 *   c1
 *   PROFILE_THIS_BLOCK(c2)
 *   c2
 * }
 *
 *
 * {
 *   PROFILE_THIS_BLOCK(c1_c2)
 *   c1
 *   PROFILE_THIS_BLOCK(c2)
 *   c2
 * }
 *
 *
 * {
 *   {
 *     PROFILE_THIS_BLOCK(c1)
 *     c1
 *   }
 *   PROFILE_THIS_BLOCK(c2)
 *   c2
 * }
 *
 *
 * {
 *   PROFILE_THIS_BLOCK(c1_c2_c3)
 *   c1
 *   {
 *     PROFILE_THIS_BLOCK(c2)
 *     c2
 *   }
 *   c3
 * }
 *
 * Influence of identifier names: they are mainly used for display
 * purpose, but the same name should not be used twice in the same
 * scope.
 *
 * {
 *   PROFILE_THIS_BLOCK(foo)
 *   c1
 *   PROFILE_THIS_BLOCK(foo) // error: identifier clash
 *   c2
 * }
 *
 * In the example below, c1+c2 and c2 are counted separately, but in
 * the output, both are confusingly labelled `foo'.
 *
 * {
 *   PROFILE_THIS_BLOCK(foo)
 *   c1
 *   {
 *     PROFILE_THIS_BLOCK(foo) // error: identifier clash
 *     c2
 *   }
 * }

 */

#ifndef PMPROF_H
#define PMPROF_H

#if defined(DISABLE_PMPROF)

// Make pmprof an empty shell
#define PROFILE_THIS_BLOCK(a)
#define PROFILE_CACHE_MISS(a)

#else

#include <chrono>
#include <iomanip>
#include <iostream>


#if defined(__GNUG__) && defined(_GLIBCXX_DEBUG)
#error Profiling is not usable when run-time debugging is in effect
#endif

namespace _pmprof {

using namespace std;
using namespace chrono;

namespace {

void dump_time(system_clock::duration value)
{
	auto musec = duration_cast<microseconds>(value).count();
	cerr << fixed << setprecision(2);
	if (musec >= 1000000)
		cerr << musec / 1000000.0 << " s";
	else if (musec >= 1000)
		cerr << musec / 1000.0 << " ms";
	else
		cerr << musec << " us";
}

void dump(system_clock::duration total, unsigned long long count) {
	if (count > 0) {
		dump_time(total / count);
		cerr << ", count=" << count << ", total=";
		dump_time(total);
	} else
		std::cerr << "no data";
	cerr << endl;
}

} // namespace


/* Helper class for gathering data. Instantiate this as a static
 * variable, so that its destructor will be executed when the program
 * ends.
 */
class stat {
public:
	stat(char const * name) : name_(name) {}

	~stat() {
		if (count_ + miss_count_ > 0) {
			if (miss_count_ == 0) {
				cerr << "#pmprof# " << name_ << ": ";
				dump(dur_, count_);
			}
			else {
				cerr << "#pmprof# " << name_ << ": ";
				dump(dur_ + miss_dur_, count_ + miss_count_);
				cerr << "   hit: " << 100 * count_ / (count_ + miss_count_) << "%, ";
				dump(dur_, count_);
				cerr << "  miss: " << 100 * miss_count_ / (count_ + miss_count_) << "%, ";
				dump(miss_dur_, miss_count_);
			}
		} else
			std::cerr << "#pmprof# " << name_ << ": no data" << std::endl;
	}

	void add(system_clock::duration d, const bool hit) {
		if (hit) {
			dur_ += d;
			count_++;
		} else {
			miss_dur_ += d;
			miss_count_++;
		}
	}

private:
	char const * name_;
	system_clock::duration dur_, miss_dur_;
	unsigned long long count_ = 0, miss_count_ = 0;
};


/* Helper class which gathers data at the end of the scope. One
 * instance of this one should be created at each execution of the
 * block. At the end of the block, it sends statistics to the static
 * stat object.
 */
class instance {
public:
	instance(stat * stat) : hit(true), stat_(stat)
	{
		before_ = system_clock::now();
	}

	~instance() {
		stat_->add(system_clock::now() - before_, hit);
	}

	bool hit;

private:
	system_clock::time_point before_;
	stat * stat_;
};

}

#define PROFILE_THIS_BLOCK(a) \
	static _pmprof::stat _pmps_##a(#a);	\
	_pmprof::instance _pmpi_##a(&_pmps_##a);

#define PROFILE_CACHE_MISS(a) \
	_pmpi_##a.hit = false;

#endif // !defined(DISABLE_PMPROF)

#endif
