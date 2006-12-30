/**
 * \file qt_connection_check.h
 * 
 * \author Peter Kümmel
 *
 * Permission to use, copy, modify, distribute and sell this software for any 
 *     purpose is hereby granted without fee, provided that the above copyright 
 *     notice appear in all copies and that both that copyright notice and this 
 *     permission notice appear in supporting documentation.
 * The author makes no representations about the 
 *     suitability of this software for any purpose. It is provided "as is" 
 *     without express or implied warranty.
 *
 */
#ifndef QT_CONNECTION_CHECK_H
#define QT_CONNECTION_CHECK_H


/*

Usage:
src/corelib/kernel/qobjectdefs.h must be patched.
Add this at the end before the endif to the header:

#ifdef signals
# ifdef QT_CONNECTION_SIGNATURE_CHECK
#  undef signals
#  define signals public
# endif
#endif

#ifdef Q_SIGNALS
# ifdef QT_CONNECTION_SIGNATURE_CHECK
#  undef Q_SIGNALS
#  define Q_SIGNALS public
# endif
#endif
*/



template<	class SIGNAL_ARG_1 = void,
			class SIGNAL_ARG_2 = void,
			class SIGNAL_ARG_3 = void,
			class SIGNAL_ARG_4 = void>
struct Wrong_Signal_Signature
{	
	template<class T>
	static void signature(T*, void (T::*  )(SIGNAL_ARG_1,
											SIGNAL_ARG_2,
											SIGNAL_ARG_3,
											SIGNAL_ARG_4) ){}	

	template<class T>
	static void existance(T*)
	{
		typedef SIGNAL_ARG_1 T1;
		typedef SIGNAL_ARG_2 T2;
		typedef SIGNAL_ARG_3 T3;
		typedef SIGNAL_ARG_4 T4;
	}	
};


template<	class SIGNAL_ARG_1,
			class SIGNAL_ARG_2,
			class SIGNAL_ARG_3>
struct Wrong_Signal_Signature<	SIGNAL_ARG_1,
								SIGNAL_ARG_2,
								SIGNAL_ARG_3,
								void>
{
	template<class T>
	static void signature(T*, void (T::*  )(SIGNAL_ARG_1,
											SIGNAL_ARG_2,
											SIGNAL_ARG_3) ){}
	template<class T>
	static void existance(T*)
	{
		typedef SIGNAL_ARG_1 T1;
		typedef SIGNAL_ARG_2 T2;
		typedef SIGNAL_ARG_3 T3;
	}	
};

template<	class SIGNAL_ARG_1,
			class SIGNAL_ARG_2>
struct Wrong_Signal_Signature<	SIGNAL_ARG_1,
								SIGNAL_ARG_2,
								void,
								void>
{
	template<class T>
	static void signature(T*, void (T::*  )(SIGNAL_ARG_1,
											SIGNAL_ARG_2) ){}
	template<class T>
	static void existance(T*)
	{
		typedef SIGNAL_ARG_1 T1;
		typedef SIGNAL_ARG_2 T2;
	}	
};


template<	class SIGNAL_ARG_1>
struct Wrong_Signal_Signature<	SIGNAL_ARG_1,
								void,
								void,
								void>
{
	template<class T>
	static void signature(T*, void (T::*  )(SIGNAL_ARG_1) ){}

	template<class T>
	static void existance(T*)
	{
		typedef SIGNAL_ARG_1 T1;
	}	
};

template<>
struct Wrong_Signal_Signature<void,void,void,void>
{
	template<class T>
	static void signature(T*, void (T::*  )()){}

	template<class T>
	static void existance(T*) {}	
};

template<	class SLOT_ARG_1 = void,
			class SLOT_ARG_2 = void,
			class SLOT_ARG_3 = void,
			class SLOT_ARG_4 = void>
struct Wrong__Slot__Signature
{
	template<class T>
	static void signature(T*, void (T::*  )(SLOT_ARG_1,
											SLOT_ARG_2,
											SLOT_ARG_3,
											SLOT_ARG_4) ){}	
	template<class T>
	static void existance(T*)
	{
		typedef SLOT_ARG_1 T1;
		typedef SLOT_ARG_2 T2;
		typedef SLOT_ARG_3 T3;
		typedef SLOT_ARG_4 T4;
	}	
};


template<	class SLOT_ARG_1,
			class SLOT_ARG_2,
			class SLOT_ARG_3>
struct Wrong__Slot__Signature<	SLOT_ARG_1,
								SLOT_ARG_2,
								SLOT_ARG_3,
								void>
{
	template<class T>
	static void signature(T*, void (T::*  )(SLOT_ARG_1,
											SLOT_ARG_2,
											SLOT_ARG_3)){}
	template<class T>
	static void existance(T*)
	{
		typedef SLOT_ARG_1 T1;
		typedef SLOT_ARG_2 T2;
		typedef SLOT_ARG_3 T3;
	}	
};

template<	class SLOT_ARG_1,
			class SLOT_ARG_2>
struct Wrong__Slot__Signature<	SLOT_ARG_1,
								SLOT_ARG_2,
								void,
								void>
{
	template<class T>
	static void signature(T*, void (T::*  )(SLOT_ARG_1,
											SLOT_ARG_2)){}
	template<class T>
	static void existance(T*)
	{
		typedef SLOT_ARG_1 T1;
		typedef SLOT_ARG_2 T2;
	}	
};

template<	class SLOT_ARG_1>
struct Wrong__Slot__Signature<	SLOT_ARG_1,
								void,
								void,
								void>
{
	template<class T>
	static void signature(T*, void (T::*  )(SLOT_ARG_1)){}	

	template<class T>
	static void existance(T*)
	{
		typedef SLOT_ARG_1 T1;
	}	
};

template<>
struct Wrong__Slot__Signature<void,void,void,void>
{
	template<class T>
	static void signature(T*, void (T::*  )()){}	

	template<class T>
	static void existance(T*){}	
};


template<class SIGNAL_ARG_TYPE, class SLOT_ARG_TYPE>
struct Signal_Slot_argument_mismatch
{
	// remove '&' from the type
	template<class T>struct RefType          {typedef T Result;};
	template<class T>struct RefType<T&>      {typedef T Result;};
	template<class T>struct RefType<const T&>{typedef T const Result;};

	template<class SIGNAL_SLOT_ARGUMENT_MISMATCH>
	static void signal_slot_arguments_mismatch(SIGNAL_SLOT_ARGUMENT_MISMATCH*) {}
	static void match()
	{
		RefType<SLOT_ARG_TYPE>::Result *p(0);
		signal_slot_arguments_mismatch<RefType<SIGNAL_ARG_TYPE>::Result>(p);
	}
};

template<class SI>
struct Signal_Slot_argument_mismatch<SI, void>
{
	static void match() {}
};

template<>
struct Signal_Slot_argument_mismatch<void, void>
{
	static void match() {}
};

template<class TOO_MUCH_SLOT_ARGUMENTS>
struct Signal_Slot_argument_mismatch<void, TOO_MUCH_SLOT_ARGUMENTS>
{
	template<class T>
	void to_much_slot_aruments(TOO_MUCH_SLOT_ARGUMENTS*)
	{}
	static void match() 
	{
		TOO_MUCH_SLOT_ARGUMENTS *p(0);
		to_much_slot_aruments<TOO_MUCH_SLOT_ARGUMENTS>(&p);
	}
};


#undef QT_CONNECTION_SIGNATURE_CHECK
#ifdef QT_CONNECTION_SIGNATURE_CHECK

#define Q_CONNECT_0_CHECK(	SI_HOST, PTR_SI, SI_NAME, \
							SL_HOST, PTR_SL, SL_NAME) \
	Wrong_Signal_Signature<void>::signature<SI_HOST>(PTR_SI,&SI_HOST::SI_NAME); \
	Wrong__Slot__Signature<void>::signature<SL_HOST>(PTR_SL,&SL_HOST::SL_NAME);

#define Q_CONNECT_1_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, \
							SL_HOST, PTR_SL, SL_NAME, SL_ARG_1) \
	Wrong_Signal_Signature<SI_ARG_1>::signature<SI_HOST>(PTR_SI,&SI_HOST::SI_NAME); \
	Wrong__Slot__Signature<SL_ARG_1>::signature<SL_HOST>(PTR_SL,&SL_HOST::SL_NAME); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_1>::match();

#define Q_CONNECT_2_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, \
							SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2) \
	Wrong_Signal_Signature<SI_ARG_1,SI_ARG_2>::signature<SI_HOST>(PTR_SI,&SI_HOST::SI_NAME); \
	Wrong__Slot__Signature<SL_ARG_1,SL_ARG_2>::signature<SL_HOST>(PTR_SL,&SL_HOST::SL_NAME); \
	Signal_Slot_argument_mismatch<SI_ARG_2,SL_ARG_2>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_1>::match();

#define Q_CONNECT_3_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, SI_ARG_3, \
							SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2, SL_ARG_3) \
	Wrong_Signal_Signature<SI_ARG_1,SI_ARG_2,SI_ARG_3>::signature<SI_HOST>(PTR_SI,&SI_HOST::SI_NAME); \
	Wrong__Slot__Signature<SL_ARG_1,SL_ARG_2,SL_ARG_3>::signature<SL_HOST>(PTR_SL,&SL_HOST::SL_NAME); \
	Signal_Slot_argument_mismatch<SI_ARG_3,SL_ARG_3>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_2,SL_ARG_2>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_1>::match();

#define Q_CONNECT_4_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, SI_ARG_3, SI_ARG_4, \
							SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2, SL_ARG_3, SL_ARG_4) \
	Wrong_Signal_Signature<SI_ARG_1,SI_ARG_2,SI_ARG_3,SI_ARG_4>::signature<SI_HOST>(PTR_SI,&SI_HOST::SI_NAME); \
	Wrong__Slot__Signature<SL_ARG_1,SL_ARG_2,SL_ARG_3,SL_ARG_4>::signature<SL_HOST>(PTR_SL,&SL_HOST::SL_NAME);
	Signal_Slot_argument_mismatch<SI_ARG_4,SL_ARG_4>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_3,SL_ARG_3>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_2,SL_ARG_2>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_1>::match();

#else

#define Q_CONNECT_0_CHECK(	SI_HOST, PTR_SI, SI_NAME, \
							SL_HOST, PTR_SL, SL_NAME) \
	Wrong_Signal_Signature<void>::existance<SI_HOST>(PTR_SI); \
	Wrong__Slot__Signature<void>::existance<SL_HOST>(PTR_SL);

#define Q_CONNECT_1_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, \
							SL_HOST, PTR_SL, SL_NAME, SL_ARG_1) \
	Wrong_Signal_Signature<SI_ARG_1>::existance<SI_HOST>(PTR_SI); \
	Wrong__Slot__Signature<SL_ARG_1>::existance<SL_HOST>(PTR_SL); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_1>::match();

#define Q_CONNECT_2_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, \
							SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2) \
	Wrong_Signal_Signature<SI_ARG_1,SI_ARG_2>::existance<SI_HOST>(PTR_SI); \
	Wrong__Slot__Signature<SL_ARG_1,SL_ARG_2>::existance<SL_HOST>(PTR_SL); \
	Signal_Slot_argument_mismatch<SI_ARG_2,SL_ARG_2>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_1>::match();

#define Q_CONNECT_3_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, SI_ARG_3, \
							SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2, SL_ARG_3) \
	Wrong_Signal_Signature<SI_ARG_1,SI_ARG_2,SI_ARG_3>::existance<SI_HOST>(PTR_SI); \
	Wrong__Slot__Signature<SL_ARG_1,SL_ARG_2,SL_ARG_3>::existance<SL_HOST>(PTR_SL); \
	Signal_Slot_argument_mismatch<SI_ARG_2,SL_ARG_3>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_2>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_1>::match();

#define Q_CONNECT_4_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, SI_ARG_3, SI_ARG_4, \
							SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2, SL_ARG_3, SL_ARG_4) \
	Wrong_Signal_Signature<SI_ARG_1,SI_ARG_2,SI_ARG_3,SI_ARG_4>::existance<SI_HOST>(PTR_SI); \
	Wrong__Slot__Signature<SL_ARG_1,SL_ARG_2,SL_ARG_3,SL_ARG_4>::existance<SL_HOST>(PTR_SL); \
	Signal_Slot_argument_mismatch<SI_ARG_4,SL_ARG_4>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_3,SL_ARG_3>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_2,SL_ARG_2>::match(); \
	Signal_Slot_argument_mismatch<SI_ARG_1,SL_ARG_1>::match();

#endif


#define Q_CONNECT_0(SI_HOST, PTR_SI, SI_NAME, \
					SL_HOST, PTR_SL, SL_NAME) \
	Q_CONNECT_0_CHECK(	SI_HOST, PTR_SI, SI_NAME,  \
						SL_HOST, PTR_SL, SL_NAME); \
	QObject::connect(	PTR_SI, SIGNAL(SI_NAME()), \
						PTR_SL, SLOT  (SL_NAME())); 

#define Q_CONNECT_1(SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, \
					SL_HOST, PTR_SL, SL_NAME, SL_ARG_1) \
	Q_CONNECT_1_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1,  \
						SL_HOST, PTR_SL, SL_NAME, SL_ARG_1); \
	QObject::connect(	PTR_SI, SIGNAL(SI_NAME(SI_ARG_1)), \
						PTR_SL, SLOT  (SL_NAME(SL_ARG_1))); 

#define Q_CONNECT_2(SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, \
					SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2) \
	Q_CONNECT_2_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2,  \
						SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2); \
	QObject::connect(	PTR_SI, SIGNAL(SI_NAME(SI_ARG_1,SI_ARG_2)), \
						PTR_SL, SLOT  (SL_NAME(SL_ARG_1,SL_ARG_2)));

#define Q_CONNECT_3(SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, SI_ARG_3, \
					SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2, SL_ARG_3) \
	Q_CONNECT_3_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, SI_ARG_3,  \
						SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2, SL_ARG_3); \
	QObject::connect(	PTR_SI, SIGNAL(SI_NAME(SI_ARG_1,SI_ARG_2,SI_ARG_3)), \
						PTR_SL, SLOT  (SL_NAME(SL_ARG_1,SL_ARG_2,SL_ARG_3)));

#define Q_CONNECT_4(SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, SI_ARG_3, SI_ARG_4, \
					SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2, SL_ARG_3, SL_ARG_4) \
	Q_CONNECT_4_CHECK(	SI_HOST, PTR_SI, SI_NAME, SI_ARG_1, SI_ARG_2, SI_ARG_3, SI_ARG_4,  \
						SL_HOST, PTR_SL, SL_NAME, SL_ARG_1, SL_ARG_2, SL_ARG_3, SL_ARG_4); \
	QObject::connect(	PTR_SI, SIGNAL(SI_NAME(SI_ARG_1,SI_ARG_2,SI_ARG_3,SI_ARG_4)), \
						PTR_SL, SLOT  (SL_NAME(SL_ARG_1,SL_ARG_2,SL_ARG_3,SL_ARG_4)));
	



#endif 
