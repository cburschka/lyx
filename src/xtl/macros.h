/*
 * Some ugly macros for XTL. Keep out!
 *
 * Copyright (C) 1998-2000 Jose' Orlando Pereira, jop@di.uminho.pt
 */
/* XTL - eXternalization Template Library - http://gsd.di.uminho.pt/~jop/xtl
 * Copyright (C) 1998-2000 Jose' Orlando Pereira, Universidade do Minho
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 *
 * Id: macros.h 1.3 Fri, 05 May 2000 18:57:58 +0100 jop 
 */

#ifndef __XTL_MACROS
#define __XTL_MACROS

// Extremely yucky pre-processor wizardry. Don't look. Just pretend it
// is not here. :-)

#define tparam(i) class T##i

#define ci_param(i) T##i& val##i
#define co_param(i) const T##i& val##i
#define c_case(i) case i:\
			simple(val##i);\
			break;

#define o_param(i) T##i* tag##i
#define o_case(i) case i:\
			reference(tag##i);\
			ptr=(Base*)tag##i;\
			break;
#define o_elseif(i) else if (typeid(*ptr)==typeid(T##i)) { \
			simple(i); \
			tag##i=(T##i*)ptr; \
			reference(tag##i); \
		}

#define tparam_2 tparam(0), tparam(1)
#define tparam_3 tparam_2, tparam(2)
#define tparam_4 tparam_3, tparam(3)
#define tparam_5 tparam_4, tparam(4)

#define X_tparam(i) tparam_##i

#define ci_param_2 ci_param(0), ci_param(1)
#define ci_param_3 ci_param_2, ci_param(2)
#define ci_param_4 ci_param_3, ci_param(3)
#define ci_param_5 ci_param_4, ci_param(4)

#define X_ci_param(i) ci_param_##i

#define co_param_2 co_param(0), co_param(1)
#define co_param_3 co_param_2, co_param(2)
#define co_param_4 co_param_3, co_param(3)
#define co_param_5 co_param_4, co_param(4)

#define X_co_param(i) co_param_##i

#define c_case_2 c_case(0) c_case(1)
#define c_case_3 c_case_2 c_case(2)
#define c_case_4 c_case_3 c_case(3)
#define c_case_5 c_case_4 c_case(4)

#define X_c_case(i) c_case_##i

#define o_param_2 o_param(0), o_param(1)
#define o_param_3 o_param_2, o_param(2)
#define o_param_4 o_param_3, o_param(3)
#define o_param_5 o_param_4, o_param(4)

#define X_o_param(i) o_param_##i

#define o_case_2 o_case(0)  o_case(1)
#define o_case_3 o_case_2 o_case(2)
#define o_case_4 o_case_3 o_case(3)
#define o_case_5 o_case_4 o_case(4)

#define X_o_case(i) o_case_##i

#define o_elseif_2 o_elseif(1)
#define o_elseif_3 o_elseif_2 o_elseif(2)
#define o_elseif_4 o_elseif_3 o_elseif(3)
#define o_elseif_5 o_elseif_4 o_elseif(4)

#define X_o_elseif(i) o_elseif_##i

#define decl_ich_method(i) \
	template <class Discr, X_tparam(i)> \
	inline obj_input& choices(Discr& discr, X_ci_param(i)) { \
		simple(discr); \
		switch(discr) { \
			X_c_case(i) \
		 default: \
		 	throw std::range_error("XTL invalid choice"); \
		}; \
		return *this; \
	}

#define decl_och_method(i) \
	template <class Discr, X_tparam(i)> \
	inline obj_output& choices(const Discr& discr, X_co_param(i)) { \
		simple(discr); \
		switch(discr) { \
			X_c_case(i) \
		 default: \
		 	throw std::range_error("XTL invalid choice"); \
		}; \
		return *this; \
	}

#define decl_iobj_method(i) \
	template <class Base, X_tparam(i)> \
	inline obj_input& object(Base*& ptr, X_o_param(i)) { \
		int discr; \
		simple(discr); \
		switch(discr) { \
			X_o_case(i) \
		 default: \
		 	throw std::range_error("XTL invalid choice"); \
		}; \
		return *this; \
	}

#define decl_oobj_method(i) \
	template <class Base, X_tparam(i)> \
	inline obj_output& object(Base*& ptr, X_o_param(i))  { \
		if (typeid(*ptr)==typeid(T0)) { \
			simple(0); \
			tag0=(T0*)ptr; \
			reference(tag0); \
		} X_o_elseif(i) \
		else \
			throw std::range_error("XTL invalid choice"); \
		return *this; \
	} 

#endif
