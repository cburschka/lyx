// -*- C++ -*-
/**
 * \file InGuiThread.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INGUITHREAD_H
#define INGUITHREAD_H

#include <QMutex>
#include <QObject>
#include <QWaitCondition>

#include "support/bind.h"
#include "support/functional.h"

namespace lyx {
namespace frontend {


class IntoGuiThreadMover : public QObject
{
	Q_OBJECT

protected:

	IntoGuiThreadMover();

	void callInGuiThread();

Q_SIGNALS:
	void triggerFunctionCall();

private Q_SLOTS:
	void doFunctionCall();

private:
	virtual void synchronousFunctionCall() = 0;

	QWaitCondition condition_;
	QMutex sync_mutex_;
};


template<class R>
class InGuiThread : private IntoGuiThreadMover
{
public:

	// please coverity by explicitly initalizing this variable.
	InGuiThread() : return_value_(R()) {}

	template<class F>
	R call(F f)
	{
		func_ = f;
		callInGuiThread();
		return return_value_;
	}

	template<class F, class P1>
	R call(F f, P1& p1)
	{
		return call(lyx::bind(f, lyx::ref(p1)));
	}

	template<class F, class P1, class P2>
	R call(F f, P1& p1, P2& p2)
	{
		return call(lyx::bind(f, lyx::ref(p1), lyx::ref(p2)));
	}

	template<class F, class P1, class P2, class P3>
	R call(F f, P1& p1, P2& p2, P3& p3)
	{
		return call(lyx::bind(f, lyx::ref(p1), lyx::ref(p2), lyx::ref(p3)));
	}

	template<class F, class P1, class P2, class P3, class P4>
	R call(F f, P1& p1, P2& p2, P3& p3, P4& p4)
	{
		return call(lyx::bind(f, lyx::ref(p1), lyx::ref(p2), lyx::ref(p3), lyx::ref(p4)));
	}

	/*
	  ...
	*/

	template<class F, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
	R call(F f, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7, P8& p8)
	{
		return call(lyx::bind(f, lyx::ref(p1), lyx::ref(p2), lyx::ref(p3), lyx::ref(p4), 
			lyx::ref(p5), lyx::ref(p6), lyx::ref(p7), lyx::ref(p8)));
	}

private:

	void synchronousFunctionCall()
	{
		return_value_ = func_();
	}

private:
	R return_value_;
	function<R()> func_;
};


// void specialisation
template<>
class InGuiThread<void> : private IntoGuiThreadMover
{
public:

	InGuiThread() {}

	template<class F>
	void call(F f)
	{
		func_ = f;
		callInGuiThread();
	}

	template<class F, class P1>
	void call(F f, P1& p1)
	{
		call(lyx::bind(f, lyx::ref(p1)));
	}

	template<class F, class P1, class P2>
	void call(F f, P1& p1, P2& p2)
	{
		call(lyx::bind(f, lyx::ref(p1), lyx::ref(p2)));
	}

	template<class F, class P1, class P2, class P3>
	void call(F f, P1& p1, P2& p2, P3& p3)
	{
		call(lyx::bind(f, lyx::ref(p1), lyx::ref(p2), lyx::ref(p3)));
	}

	template<class F, class P1, class P2, class P3, class P4>
	void call(F f, P1& p1, P2& p2, P3& p3, P4& p4)
	{
		call(lyx::bind(f, lyx::ref(p1), lyx::ref(p2), lyx::ref(p3), lyx::ref(p4)));
	}

	/*
	  ...
	*/

	template<class F, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
	void call(F f, P1& p1, P2& p2, P3& p3, P4& p4, P5& p5, P6& p6, P7& p7, P8& p8)
	{
		call(bind(f, lyx::ref(p1), lyx::ref(p2), lyx::ref(p3), lyx::ref(p4), lyx::ref(p5),
			lyx::ref(p6), lyx::ref(p7), lyx::ref(p8)));
	}

private:

	void synchronousFunctionCall()
	{
		func_();
	}

private:
	function<void()> func_;
};


} // namespace frontend
} // namespace lyx

#endif // INGUITHREAD_H
