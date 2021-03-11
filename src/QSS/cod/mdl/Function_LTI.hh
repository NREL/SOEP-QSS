// Linear Time-Invariant Function
//
// Project: QSS Solver
//
// Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
// the National Renewable Energy Laboratory of the U.S. Department of Energy
//
// Copyright (c) 2017-2021 Objexx Engineering, Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
// (3) Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES
// GOVERNMENT, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef QSS_cod_mdl_Function_LTI_hh_INCLUDED
#define QSS_cod_mdl_Function_LTI_hh_INCLUDED

// QSS Headers
#include <QSS/math.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

namespace QSS {
namespace cod {
namespace mdl {

// Linear Time-Invariant Function
//
// Note: Adding many terms into a sorted vector could be a performance issue
template< typename V > // Template to avoid cyclic inclusion with Variable
class Function_LTI final
{

public: // Types

	using Coefficient = double;
	using Variable = V;

	struct Term
	{
		Coefficient c;
		Variable * v;

		// Term < Term: Partial ordering by Variable order
		friend
		bool
		operator <( Term const & l, Term const & r )
		{
			return ( l.v->order() < r.v->order() );
		}

	}; // Term
	using Terms = std::vector< Term >;
	using size_type = typename Terms::size_type;

	using Time = typename Variable::Time;
	using Real = typename Variable::Real;
	using AdvanceSpecs_LIQSS1 = typename Variable::AdvanceSpecs_LIQSS1;
	using AdvanceSpecs_LIQSS2 = typename Variable::AdvanceSpecs_LIQSS2;
	using AdvanceSpecs_LIQSS3 = typename Variable::AdvanceSpecs_LIQSS3;

public: // Property

	// Continuous Value at Time t
	Real
	operator ()( Time const t ) const
	{
		Real r( c0_ );
		for ( Term const & term : terms_ ) {
			r += term.c * term.v->x( t );
		}
		return r;
	}

	// Continuous Value at Time t
	Real
	x( Time const t ) const
	{
		Real r( c0_ );
		for ( Term const & term : terms_ ) {
			r += term.c * term.v->x( t );
		}
		return r;
	}

	// Continuous First Derivative at Time t
	Real
	x1( Time const t ) const
	{
		Real r( 0.0 );
		for ( size_type i = iBeg1, n = terms_.size(); i < n; ++i ) {
			Term const & term( terms_[ i ] );
			r += term.c * term.v->x1( t );
		}
		return r;
	}

	// Continuous Second Derivative at Time t
	Real
	x2( Time const t ) const
	{
		Real r( 0.0 );
		for ( size_type i = iBeg2, n = terms_.size(); i < n; ++i ) {
			Term const & term( terms_[ i ] );
			r += term.c * term.v->x2( t );
		}
		return r;
	}

	// Continuous Third Derivative at Time t
	Real
	x3( Time const t ) const
	{
		Real r( 0.0 );
		for ( size_type i = iBeg3, n = terms_.size(); i < n; ++i ) {
			Term const & term( terms_[ i ] );
			r += term.c * term.v->x3( t );
		}
		return r;
	}

	// Quantized Value at Time t
	Real
	q( Time const t ) const
	{
		Real r( c0_ );
		for ( Term const & term : terms_ ) {
			r += term.c * term.v->q( t );
		}
		return r;
	}

	// Quantized First Derivative at Time t
	Real
	q1( Time const t ) const
	{
		Real r( 0.0 );
		for ( size_type i = iBeg1, n = terms_.size(); i < n; ++i ) {
			Term const & term( terms_[ i ] );
			r += term.c * term.v->q1( t );
		}
		return r;
	}

	// Quantized Second Derivative at Time t
	Real
	q2( Time const t ) const
	{
		Real r( 0.0 );
		for ( size_type i = iBeg2, n = terms_.size(); i < n; ++i ) {
			Term const & term( terms_[ i ] );
			r += term.c * term.v->q2( t );
		}
		return r;
	}

	// Quantized Third Derivative at Time t
	Real
	q3( Time const t ) const
	{
		Real r( 0.0 );
		for ( size_type i = iBeg3, n = terms_.size(); i < n; ++i ) {
			Term const & term( terms_[ i ] );
			r += term.c * term.v->q3( t );
		}
		return r;
	}

	// Quantized Sequential Value at Time t
	Real
	qs( Time const t ) const
	{
		return q( t );
	}

	// Quantized Forward-Difference Sequential First Derivative at Time t
	Real
	qf1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential First Derivative at Time t
	Real
	qc1( Time const t ) const
	{
		return q1( t );
	}

	// Quantized Centered-Difference Sequential Second Derivative at Time t
	Real
	qc2( Time const t ) const
	{
		return q2( t );
	}

	// Quantized Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	qlu1( Time const t, Real const del ) const
	{
		// Value at +/- del
		Real v( c0_ );
		for ( Term const & term : termso_ ) {
			v += term.c * term.v->q( t );
		}
		Real const vc( cv_ == 0.0 ? v : v + ( cv_ * v_->q( t ) ) );
		Real const cv_del( cv_ * del );
		Real const vl( vc - cv_del );
		Real const vu( vc + cv_del );

		// Zero point
		Real const z( signum( vl ) != signum( vu ) ? -( v * cv_inv_ ) : 0.0 );

		return AdvanceSpecs_LIQSS1{ vl, vu, z };
	}

	// Quantized Values at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS1
	qlu1( Time const t, Real const del, Real const vc ) const
	{
		// Value at +/- del
		Real const v( cv_ == 0.0 ? vc : vc - ( cv_ * v_->q( t ) ) );
		Real const cv_del( cv_ * del );
		Real const vl( vc - cv_del );
		Real const vu( vc + cv_del );

		// Zero point
		Real const z( signum( vl ) != signum( vu ) ? -( v * cv_inv_ ) : 0.0 );

		return AdvanceSpecs_LIQSS1{ vl, vu, z };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	qlu2( Time const t, Real const del ) const
	{
		// Value at +/- del
		Real v( c0_ );
		for ( Term const & term : termso_ ) {
			v += term.c * term.v->q( t );
		}
		Real const vc( cv_ == 0.0 ? v : v + ( cv_ * v_->q( t ) ) );
		Real const cv_del( cv_ * del );
		Real const vl( vc - cv_del );
		Real const vu( vc + cv_del );

		// Derivative at +/- del
		Real s( 0.0 );
		for ( size_type i = ioBeg1, n = termso_.size(); i < n; ++i ) {
			Term const & term( termso_[ i ] );
			s += term.c * term.v->q1( t );
		}
		Real const sl( s + ( cv_ * vl ) );
		Real const su( s + ( cv_ * vu ) );

		// Zero point
		bool const signs_differ( signum( sl ) != signum( su ) );
		Real const z1( signs_differ ? -( s * cv_inv_ ) : 0.0 );
		Real const z0( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, sl, su, z0, z1 };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	qlu2( Time const t, Real const del, Real const vc ) const
	{
		// Value at +/- del
		Real const v( cv_ == 0.0 ? vc : vc - ( cv_ * v_->q( t ) ) );
		Real const cv_del( cv_ * del );
		Real const vl( vc - cv_del );
		Real const vu( vc + cv_del );

		// Derivative at +/- del
		Real s( 0.0 );
		for ( size_type i = ioBeg1, n = termso_.size(); i < n; ++i ) {
			Term const & term( termso_[ i ] );
			s += term.c * term.v->q1( t );
		}
		Real const sl( s + ( cv_ * vl ) );
		Real const su( s + ( cv_ * vu ) );

		// Zero point
		bool const signs_differ( signum( sl ) != signum( su ) );
		Real const z1( signs_differ ? -( s * cv_inv_ ) : 0.0 );
		Real const z0( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, sl, su, z0, z1 };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS2
	qlu2( Time const t, Real const del, Real const vc, Real const sc ) const
	{
		// Value at +/- del
		Real const v( cv_ == 0.0 ? vc : vc - ( cv_ * v_->q( t ) ) );
		Real const cv_del( cv_ * del );
		Real const vl( vc - cv_del );
		Real const vu( vc + cv_del );

		// Derivative at +/- del
		Real const s( cv_ == 0.0 ? sc : sc - ( cv_ * v_->q1( t ) ) );
		Real const sl( s + ( cv_ * vl ) );
		Real const su( s + ( cv_ * vu ) );

		// Zero point
		bool const signs_differ( signum( sl ) != signum( su ) );
		Real const z1( signs_differ ? -( s * cv_inv_ ) : 0.0 );
		Real const z0( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS2{ vl, vu, sl, su, z0, z1 };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS3
	qlu3( Time const t, Real const del ) const
	{
		// Value at +/- del
		Real v( c0_ );
		for ( Term const & term : termso_ ) {
			v += term.c * term.v->q( t );
		}
		Real const vc( cv_ == 0.0 ? v : v + ( cv_ * v_->q( t ) ) );
		Real const cv_del( cv_ * del );
		Real const vl( vc - cv_del );
		Real const vu( vc + cv_del );

		// Derivative at +/- del
		Real s( 0.0 );
		for ( size_type i = ioBeg1, n = termso_.size(); i < n; ++i ) {
			Term const & term( termso_[ i ] );
			s += term.c * term.v->q1( t );
		}
		Real const sl( s + ( cv_ * vl ) );
		Real const su( s + ( cv_ * vu ) );

		// 2nd derivative at +/- del
		Real c( 0.0 );
		for ( size_type i = ioBeg2, n = termso_.size(); i < n; ++i ) {
			Term const & term( termso_[ i ] );
			c += term.c * term.v->q2( t );
		}
		Real const cl( c + ( cv_ * sl ) );
		Real const cu( c + ( cv_ * su ) );

		// Zero point
		bool const signs_differ( signum( cl ) != signum( cu ) );
		Real const z2( signs_differ ? -( c * cv_inv_ ) : 0.0 );
		Real const z1( signs_differ ? ( z2 - s ) * cv_inv_ : 0.0 );
		Real const z0( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS3{ vl, vu, sl, su, cl, cu, z0, z1, z2 };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS3
	qlu3( Time const t, Real const del, Real const vc, Real const sc ) const
	{
		// Value at +/- del
		Real const v( cv_ == 0.0 ? vc : vc - ( cv_ * v_->q( t ) ) );
		Real const cv_del( cv_ * del );
		Real const vl( vc - cv_del );
		Real const vu( vc + cv_del );

		// Derivative at +/- del
		Real const s( cv_ == 0.0 ? sc : sc - ( cv_ * v_->q1( t ) ) );
		Real const sl( s + ( cv_ * vl ) );
		Real const su( s + ( cv_ * vu ) );

		// 2nd derivative at +/- del
		Real c( 0.0 );
		for ( size_type i = ioBeg2, n = termso_.size(); i < n; ++i ) {
			Term const & term( termso_[ i ] );
			c += term.c * term.v->q2( t );
		}
		Real const cl( c + ( cv_ * sl ) );
		Real const cu( c + ( cv_ * su ) );

		// Zero point
		bool const signs_differ( signum( cl ) != signum( cu ) );
		Real const z2( signs_differ ? -( c * cv_inv_ ) : 0.0 );
		Real const z1( signs_differ ? ( z2 - s ) * cv_inv_ : 0.0 );
		Real const z0( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS3{ vl, vu, sl, su, cl, cu, z0, z1, z2 };
	}

	// Quantized Values and Derivatives at Time t and at Variable +/- Delta
	AdvanceSpecs_LIQSS3
	qlu3( Time const t, Real const del, Real const vc, Real const sc, Real const cc ) const
	{
		// Value at +/- del
		Real const v( cv_ == 0.0 ? vc : vc - ( cv_ * v_->q( t ) ) );
		Real const cv_del( cv_ * del );
		Real const vl( vc - cv_del );
		Real const vu( vc + cv_del );

		// Derivative at +/- del
		Real const s( cv_ == 0.0 ? sc : sc - ( cv_ * v_->q1( t ) ) );
		Real const sl( s + ( cv_ * vl ) );
		Real const su( s + ( cv_ * vu ) );

		// 2nd derivative at +/- del
		Real const c( cv_ == 0.0 ? cc : cc - ( cv_ * v_->q2( t ) ) );
		Real const cl( c + ( cv_ * sl ) );
		Real const cu( c + ( cv_ * su ) );

		// Zero point
		bool const signs_differ( signum( cl ) != signum( cu ) );
		Real const z2( signs_differ ? -( c * cv_inv_ ) : 0.0 );
		Real const z1( signs_differ ? ( z2 - s ) * cv_inv_ : 0.0 );
		Real const z0( signs_differ ? ( z1 - v ) * cv_inv_ : 0.0 );

		return AdvanceSpecs_LIQSS3{ vl, vu, sl, su, cl, cu, z0, z1, z2 };
	}

public: // Methods

	// Add Constant
	void
	add( Coefficient const c0 )
	{
		c0_ = c0;
	}

	// Add Variable
	void
	add( Variable * v, Variable * self = nullptr )
	{
		assert( v != nullptr );
		Term term{ 1.0, v };
		if ( v == self ) { // Self Variable
			v_ = v; // Register self Variable
			terms_.insert( std::upper_bound( terms_.begin(), terms_.end(), term ), std::move( term ) );
			switch ( v->order() ) {
			case 0:
				++iBeg1;
#if (__cplusplus >= 201703L) //C++17
				[[fallthrough]];
#elif defined(__clang__)
				[[clang::fallthrough]];
#elif defined(__GNUC__)
				[[gnu::fallthrough]];
#endif
			case 1:
				++iBeg2;
#if (__cplusplus >= 201703L) //C++17
				[[fallthrough]];
#elif defined(__clang__)
				[[clang::fallthrough]];
#elif defined(__GNUC__)
				[[gnu::fallthrough]];
#endif
			case 2:
				++iBeg3;
			}
			cv_ = cv_inv_ = 1.0;
		} else { // Non-self Variable
			terms_.insert( std::upper_bound( terms_.begin(), terms_.end(), term ), term );
			termso_.insert( std::upper_bound( termso_.begin(), termso_.end(), term ), std::move( term ) );
			switch ( v->order() ) {
			case 0:
				++iBeg1;
				++ioBeg1;
#if (__cplusplus >= 201703L) //C++17
				[[fallthrough]];
#elif defined(__clang__)
				[[clang::fallthrough]];
#elif defined(__GNUC__)
				[[gnu::fallthrough]];
#endif
			case 1:
				++iBeg2;
				++ioBeg2;
#if (__cplusplus >= 201703L) //C++17
				[[fallthrough]];
#elif defined(__clang__)
				[[clang::fallthrough]];
#elif defined(__GNUC__)
				[[gnu::fallthrough]];
#endif
			case 2:
				++iBeg3;
				++ioBeg3;
			}
		}
	}

	// Add Coefficient * Variable
	void
	add(
	 Coefficient const c,
	 Variable * v,
	 Variable * self = nullptr
	)
	{
		assert( v != nullptr );
		Term term{ c, v };
		if ( v == self ) { // Self Variable
			v_ = v; // Register self Variable
			terms_.insert( std::upper_bound( terms_.begin(), terms_.end(), term ), std::move( term ) );
			switch ( v->order() ) {
			case 0:
				++iBeg1;
#if (__cplusplus >= 201703L) //C++17
				[[fallthrough]];
#elif defined(__clang__)
				[[clang::fallthrough]];
#elif defined(__GNUC__)
				[[gnu::fallthrough]];
#endif
			case 1:
				++iBeg2;
#if (__cplusplus >= 201703L) //C++17
				[[fallthrough]];
#elif defined(__clang__)
				[[clang::fallthrough]];
#elif defined(__GNUC__)
				[[gnu::fallthrough]];
#endif
			case 2:
				++iBeg3;
			}
			cv_ = c;
			cv_inv_ = ( cv_ != 0.0 ? 1.0 / cv_ : infinity );
		} else { // Non-self Variable
			terms_.insert( std::upper_bound( terms_.begin(), terms_.end(), term ), term );
			termso_.insert( std::upper_bound( termso_.begin(), termso_.end(), term ), std::move( term ) );
			switch ( v->order() ) {
			case 0:
				++iBeg1;
				++ioBeg1;
#if (__cplusplus >= 201703L) //C++17
				[[fallthrough]];
#elif defined(__clang__)
				[[clang::fallthrough]];
#elif defined(__GNUC__)
				[[gnu::fallthrough]];
#endif
			case 1:
				++iBeg2;
				++ioBeg2;
#if (__cplusplus >= 201703L) //C++17
				[[fallthrough]];
#elif defined(__clang__)
				[[clang::fallthrough]];
#elif defined(__GNUC__)
				[[gnu::fallthrough]];
#endif
			case 2:
				++iBeg3;
				++ioBeg3;
			}
		}
	}

private: // Data

	Coefficient c0_{ 0.0 }; // Constant term
	Terms terms_; // Coefficient * Variable terms
	size_type iBeg1{ 0u }; // Index of first Variable of QSS orders 1+
	size_type iBeg2{ 0u }; // Index of first Variable of QSS orders 2+
	size_type iBeg3{ 0u }; // Index of first Variable of QSS orders 3+
	size_type ioBeg1{ 0u }; // Index of first non-self Variable of QSS order 1+
	size_type ioBeg2{ 0u }; // Index of first non-self Variable of QSS order 2+
	size_type ioBeg3{ 0u }; // Index of first non-self Variable of QSS order 3+

	Variable * v_{ nullptr }; // Self Variable
	Coefficient cv_{ 0.0 }; // Coefficient of self Variable
	Coefficient cv_inv_{ 0.0 }; // Inverse of coefficient of self Variable
	Terms termso_; // Coefficient * Variable terms for non-self Variables

};

} // mdl
} // cod
} // QSS

#endif
