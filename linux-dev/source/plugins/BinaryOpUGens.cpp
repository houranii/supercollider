/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "SC_PlugIn.h"

static InterfaceTable *ft;

//////////////////////////////////////////////////////////////////////////////////////////////////


/* special binary math operators */
enum {
	opAdd,		
	opSub, 
	opMul, 
	opIDiv, 
	opFDiv, 
	opMod,
	opEQ,		
	opNE, 
	opLT, 
	opGT, 
	opLE,
	opGE,
	//opIdentical,
	//opNotIdentical,	
	
	opMin, 
	opMax,
	opBitAnd,
	opBitOr,
	opBitXor,	
	opLCM,
	opGCD,
	opRound,
	opTrunc,
	opAtan2,	
	opHypot,
	opHypotx,
	opPow,
	opShiftLeft, //
	opShiftRight, //
	opUnsignedShift, //
	opFill, //
	opRing1,	// a * (b + 1) == a * b + a
	opRing2,	// a * b + a + b
	opRing3,	// a*a*b
	opRing4,	// a*a*b - a*b*b
	opDifSqr,	// a*a - b*b
	opSumSqr,	// a*a + b*b
	opSqrSum,	// (a + b)^2
	opSqrDif,	// (a - b)^2
	opAbsDif,	// |a - b|
	opThresh,
	opAMClip,
	opScaleNeg,
	opClip2,
	opExcess,
	opFold2,
	opWrap2,
	opFirstArg,
	opRandRange,
	opExpRandRange,
	
	opNumBinarySelectors
};


inline float sc_andt(float a, float b)
{
	return a > 0.f && b > 0.f ? 1.f : 0.f;
}

inline float sc_ort(float a, float b)
{
	return a > 0.f || b > 0.f ? 1.f : 0.f;
}

inline float sc_xort(float a, float b)
{
	return a > 0.f ? (b > 0.f ? 0.f : 1.f) : (b > 0.f ? 1.f : 0.f);
}

struct BinaryOpUGen : public Unit
{
	float mPrevA, mPrevB;
};

typedef void (*BinaryOpFunc)(BinaryOpUGen *unit, int inNumSamples);

extern "C"
{
	void load(InterfaceTable *inTable);

	void BinaryOpUGen_Ctor(BinaryOpUGen *unit);

	void zero_1(BinaryOpUGen *unit, int inNumSamples);
	void zero_aa(BinaryOpUGen *unit, int inNumSamples);
	void firstarg_1(BinaryOpUGen *unit, int inNumSamples);
	void firstarg_aa(BinaryOpUGen *unit, int inNumSamples);
	void secondarg_1(BinaryOpUGen *unit, int inNumSamples);
	void secondarg_aa(BinaryOpUGen *unit, int inNumSamples);
	void add_1(BinaryOpUGen *unit, int inNumSamples);
	void add_aa(BinaryOpUGen *unit, int inNumSamples);
	void add_ak(BinaryOpUGen *unit, int inNumSamples);
	void add_ka(BinaryOpUGen *unit, int inNumSamples);
	void add_ai(BinaryOpUGen *unit, int inNumSamples);
	void add_ia(BinaryOpUGen *unit, int inNumSamples);
	void sub_1(BinaryOpUGen *unit, int inNumSamples);
	void sub_aa(BinaryOpUGen *unit, int inNumSamples);
	void sub_ak(BinaryOpUGen *unit, int inNumSamples);
	void sub_ka(BinaryOpUGen *unit, int inNumSamples);
	void sub_ai(BinaryOpUGen *unit, int inNumSamples);
	void sub_ia(BinaryOpUGen *unit, int inNumSamples);
	void mul_1(BinaryOpUGen *unit, int inNumSamples);
	void mul_aa(BinaryOpUGen *unit, int inNumSamples);
	void mul_ak(BinaryOpUGen *unit, int inNumSamples);
	void mul_ka(BinaryOpUGen *unit, int inNumSamples);
	void mul_ai(BinaryOpUGen *unit, int inNumSamples);
	void mul_ia(BinaryOpUGen *unit, int inNumSamples);
	void div_1(BinaryOpUGen *unit, int inNumSamples);
	void div_aa(BinaryOpUGen *unit, int inNumSamples);
	void div_ak(BinaryOpUGen *unit, int inNumSamples);
	void div_ka(BinaryOpUGen *unit, int inNumSamples);
	void div_ai(BinaryOpUGen *unit, int inNumSamples);
	void div_ia(BinaryOpUGen *unit, int inNumSamples);
	void mod_1(BinaryOpUGen *unit, int inNumSamples);
	void mod_aa(BinaryOpUGen *unit, int inNumSamples);
	void mod_ak(BinaryOpUGen *unit, int inNumSamples);
	void mod_ka(BinaryOpUGen *unit, int inNumSamples);
	void mod_ai(BinaryOpUGen *unit, int inNumSamples);
	void mod_ia(BinaryOpUGen *unit, int inNumSamples);
	void max_1(BinaryOpUGen *unit, int inNumSamples);
	void max_aa(BinaryOpUGen *unit, int inNumSamples);
	void max_ak(BinaryOpUGen *unit, int inNumSamples);
	void max_ka(BinaryOpUGen *unit, int inNumSamples);
	void max_ai(BinaryOpUGen *unit, int inNumSamples);
	void max_ia(BinaryOpUGen *unit, int inNumSamples);
	void min_1(BinaryOpUGen *unit, int inNumSamples);
	void min_aa(BinaryOpUGen *unit, int inNumSamples);
	void min_ak(BinaryOpUGen *unit, int inNumSamples);
	void min_ka(BinaryOpUGen *unit, int inNumSamples);
	void min_ai(BinaryOpUGen *unit, int inNumSamples);
	void min_ia(BinaryOpUGen *unit, int inNumSamples);

	void and_1(BinaryOpUGen *unit, int inNumSamples);
	void and_aa(BinaryOpUGen *unit, int inNumSamples);
	void and_ak(BinaryOpUGen *unit, int inNumSamples);
	void and_ka(BinaryOpUGen *unit, int inNumSamples);
	void and_ai(BinaryOpUGen *unit, int inNumSamples);
	void and_ia(BinaryOpUGen *unit, int inNumSamples);

	void or_1(BinaryOpUGen *unit, int inNumSamples);
	void or_aa(BinaryOpUGen *unit, int inNumSamples);
	void or_ak(BinaryOpUGen *unit, int inNumSamples);
	void or_ka(BinaryOpUGen *unit, int inNumSamples);
	void or_ai(BinaryOpUGen *unit, int inNumSamples);
	void or_ia(BinaryOpUGen *unit, int inNumSamples);

	void xor_1(BinaryOpUGen *unit, int inNumSamples);
	void xor_aa(BinaryOpUGen *unit, int inNumSamples);
	void xor_ak(BinaryOpUGen *unit, int inNumSamples);
	void xor_ka(BinaryOpUGen *unit, int inNumSamples);
	void xor_ai(BinaryOpUGen *unit, int inNumSamples);
	void xor_ia(BinaryOpUGen *unit, int inNumSamples);

	void amclip_1(BinaryOpUGen *unit, int inNumSamples);
	void amclip_aa(BinaryOpUGen *unit, int inNumSamples);
	void amclip_ak(BinaryOpUGen *unit, int inNumSamples);
	void amclip_ka(BinaryOpUGen *unit, int inNumSamples);
	void amclip_ai(BinaryOpUGen *unit, int inNumSamples);
	void amclip_ia(BinaryOpUGen *unit, int inNumSamples);
	void scaleneg_1(BinaryOpUGen *unit, int inNumSamples);
	void scaleneg_aa(BinaryOpUGen *unit, int inNumSamples);
	void scaleneg_ak(BinaryOpUGen *unit, int inNumSamples);
	void scaleneg_ka(BinaryOpUGen *unit, int inNumSamples);
	void scaleneg_ai(BinaryOpUGen *unit, int inNumSamples);
	void scaleneg_ia(BinaryOpUGen *unit, int inNumSamples);
	void pow_1(BinaryOpUGen *unit, int inNumSamples);
	void pow_aa(BinaryOpUGen *unit, int inNumSamples);
	void pow_ak(BinaryOpUGen *unit, int inNumSamples);
	void pow_ka(BinaryOpUGen *unit, int inNumSamples);
	void pow_ai(BinaryOpUGen *unit, int inNumSamples);
	void pow_ia(BinaryOpUGen *unit, int inNumSamples);
	void ring1_1(BinaryOpUGen *unit, int inNumSamples);
	void ring1_aa(BinaryOpUGen *unit, int inNumSamples);
	void ring1_ak(BinaryOpUGen *unit, int inNumSamples);
	void ring1_ka(BinaryOpUGen *unit, int inNumSamples);
	void ring1_ai(BinaryOpUGen *unit, int inNumSamples);
	void ring1_ia(BinaryOpUGen *unit, int inNumSamples);
	void ring2_1(BinaryOpUGen *unit, int inNumSamples);
	void ring2_aa(BinaryOpUGen *unit, int inNumSamples);
	void ring2_ak(BinaryOpUGen *unit, int inNumSamples);
	void ring2_ka(BinaryOpUGen *unit, int inNumSamples);
	void ring2_ai(BinaryOpUGen *unit, int inNumSamples);
	void ring2_ia(BinaryOpUGen *unit, int inNumSamples);
	void ring3_1(BinaryOpUGen *unit, int inNumSamples);
	void ring3_aa(BinaryOpUGen *unit, int inNumSamples);
	void ring3_ak(BinaryOpUGen *unit, int inNumSamples);
	void ring3_ka(BinaryOpUGen *unit, int inNumSamples);
	void ring3_ai(BinaryOpUGen *unit, int inNumSamples);
	void ring3_ia(BinaryOpUGen *unit, int inNumSamples);
	void ring4_1(BinaryOpUGen *unit, int inNumSamples);
	void ring4_aa(BinaryOpUGen *unit, int inNumSamples);
	void ring4_ak(BinaryOpUGen *unit, int inNumSamples);
	void ring4_ka(BinaryOpUGen *unit, int inNumSamples);
	void ring4_ai(BinaryOpUGen *unit, int inNumSamples);
	void ring4_ia(BinaryOpUGen *unit, int inNumSamples);
	void thresh_1(BinaryOpUGen *unit, int inNumSamples);
	void thresh_aa(BinaryOpUGen *unit, int inNumSamples);
	void thresh_ak(BinaryOpUGen *unit, int inNumSamples);
	void thresh_ka(BinaryOpUGen *unit, int inNumSamples);
	void thresh_ai(BinaryOpUGen *unit, int inNumSamples);
	void thresh_ia(BinaryOpUGen *unit, int inNumSamples);
	void clip2_1(BinaryOpUGen *unit, int inNumSamples);
	void clip2_aa(BinaryOpUGen *unit, int inNumSamples);
	void clip2_ak(BinaryOpUGen *unit, int inNumSamples);
	void clip2_ka(BinaryOpUGen *unit, int inNumSamples);
	void clip2_ai(BinaryOpUGen *unit, int inNumSamples);
	void clip2_ia(BinaryOpUGen *unit, int inNumSamples);
	void fold2_1(BinaryOpUGen *unit, int inNumSamples);
	void fold2_aa(BinaryOpUGen *unit, int inNumSamples);
	void fold2_ak(BinaryOpUGen *unit, int inNumSamples);
	void fold2_ka(BinaryOpUGen *unit, int inNumSamples);
	void fold2_ai(BinaryOpUGen *unit, int inNumSamples);
	void fold2_ia(BinaryOpUGen *unit, int inNumSamples);
	void wrap2_1(BinaryOpUGen *unit, int inNumSamples);
	void wrap2_aa(BinaryOpUGen *unit, int inNumSamples);
	void wrap2_ak(BinaryOpUGen *unit, int inNumSamples);
	void wrap2_ka(BinaryOpUGen *unit, int inNumSamples);
	void wrap2_ai(BinaryOpUGen *unit, int inNumSamples);
	void wrap2_ia(BinaryOpUGen *unit, int inNumSamples);
	void excess_1(BinaryOpUGen *unit, int inNumSamples);
	void excess_aa(BinaryOpUGen *unit, int inNumSamples);
	void excess_ak(BinaryOpUGen *unit, int inNumSamples);
	void excess_ka(BinaryOpUGen *unit, int inNumSamples);
	void excess_ai(BinaryOpUGen *unit, int inNumSamples);
	void excess_ia(BinaryOpUGen *unit, int inNumSamples);
	void lt_1(BinaryOpUGen *unit, int inNumSamples);
	void lt_aa(BinaryOpUGen *unit, int inNumSamples);
	void lt_ak(BinaryOpUGen *unit, int inNumSamples);
	void lt_ka(BinaryOpUGen *unit, int inNumSamples);
	void lt_ai(BinaryOpUGen *unit, int inNumSamples);
	void lt_ia(BinaryOpUGen *unit, int inNumSamples);
	void le_1(BinaryOpUGen *unit, int inNumSamples);
	void le_aa(BinaryOpUGen *unit, int inNumSamples);
	void le_ak(BinaryOpUGen *unit, int inNumSamples);
	void le_ka(BinaryOpUGen *unit, int inNumSamples);
	void le_ai(BinaryOpUGen *unit, int inNumSamples);
	void le_ia(BinaryOpUGen *unit, int inNumSamples);
	void gt_1(BinaryOpUGen *unit, int inNumSamples);
	void gt_aa(BinaryOpUGen *unit, int inNumSamples);
	void gt_ak(BinaryOpUGen *unit, int inNumSamples);
	void gt_ka(BinaryOpUGen *unit, int inNumSamples);
	void gt_ai(BinaryOpUGen *unit, int inNumSamples);
	void gt_ia(BinaryOpUGen *unit, int inNumSamples);
	void ge_1(BinaryOpUGen *unit, int inNumSamples);
	void ge_aa(BinaryOpUGen *unit, int inNumSamples);
	void ge_ak(BinaryOpUGen *unit, int inNumSamples);
	void ge_ka(BinaryOpUGen *unit, int inNumSamples);
	void ge_ai(BinaryOpUGen *unit, int inNumSamples);
	void ge_ia(BinaryOpUGen *unit, int inNumSamples);
	void eq_1(BinaryOpUGen *unit, int inNumSamples);
	void eq_aa(BinaryOpUGen *unit, int inNumSamples);
	void eq_ak(BinaryOpUGen *unit, int inNumSamples);
	void eq_ka(BinaryOpUGen *unit, int inNumSamples);
	void eq_ai(BinaryOpUGen *unit, int inNumSamples);
	void eq_ia(BinaryOpUGen *unit, int inNumSamples);
	void neq_1(BinaryOpUGen *unit, int inNumSamples);
	void neq_aa(BinaryOpUGen *unit, int inNumSamples);
	void neq_ak(BinaryOpUGen *unit, int inNumSamples);
	void neq_ka(BinaryOpUGen *unit, int inNumSamples);
	void neq_ai(BinaryOpUGen *unit, int inNumSamples);
	void neq_ia(BinaryOpUGen *unit, int inNumSamples);
	void sumsqr_1(BinaryOpUGen *unit, int inNumSamples);
	void sumsqr_aa(BinaryOpUGen *unit, int inNumSamples);
	void sumsqr_ak(BinaryOpUGen *unit, int inNumSamples);
	void sumsqr_ka(BinaryOpUGen *unit, int inNumSamples);
	void sumsqr_ai(BinaryOpUGen *unit, int inNumSamples);
	void sumsqr_ia(BinaryOpUGen *unit, int inNumSamples);
	void difsqr_1(BinaryOpUGen *unit, int inNumSamples);
	void difsqr_aa(BinaryOpUGen *unit, int inNumSamples);
	void difsqr_ak(BinaryOpUGen *unit, int inNumSamples);
	void difsqr_ka(BinaryOpUGen *unit, int inNumSamples);
	void difsqr_ai(BinaryOpUGen *unit, int inNumSamples);
	void difsqr_ia(BinaryOpUGen *unit, int inNumSamples);
	void sqrsum_1(BinaryOpUGen *unit, int inNumSamples);
	void sqrsum_aa(BinaryOpUGen *unit, int inNumSamples);
	void sqrsum_ak(BinaryOpUGen *unit, int inNumSamples);
	void sqrsum_ka(BinaryOpUGen *unit, int inNumSamples);
	void sqrsum_ai(BinaryOpUGen *unit, int inNumSamples);
	void sqrsum_ia(BinaryOpUGen *unit, int inNumSamples);
	void sqrdif_1(BinaryOpUGen *unit, int inNumSamples);
	void sqrdif_aa(BinaryOpUGen *unit, int inNumSamples);
	void sqrdif_ak(BinaryOpUGen *unit, int inNumSamples);
	void sqrdif_ka(BinaryOpUGen *unit, int inNumSamples);
	void sqrdif_ai(BinaryOpUGen *unit, int inNumSamples);
	void sqrdif_ia(BinaryOpUGen *unit, int inNumSamples);
	void absdif_1(BinaryOpUGen *unit, int inNumSamples);
	void absdif_aa(BinaryOpUGen *unit, int inNumSamples);
	void absdif_ak(BinaryOpUGen *unit, int inNumSamples);
	void absdif_ka(BinaryOpUGen *unit, int inNumSamples);
	void absdif_ai(BinaryOpUGen *unit, int inNumSamples);
	void absdif_ia(BinaryOpUGen *unit, int inNumSamples);
	void round_1(BinaryOpUGen *unit, int inNumSamples);
	void round_aa(BinaryOpUGen *unit, int inNumSamples);
	void round_ak(BinaryOpUGen *unit, int inNumSamples);
	void round_ka(BinaryOpUGen *unit, int inNumSamples);
	void round_ai(BinaryOpUGen *unit, int inNumSamples);
	void round_ia(BinaryOpUGen *unit, int inNumSamples);
	void trunc_1(BinaryOpUGen *unit, int inNumSamples);
	void trunc_aa(BinaryOpUGen *unit, int inNumSamples);
	void trunc_ak(BinaryOpUGen *unit, int inNumSamples);
	void trunc_ka(BinaryOpUGen *unit, int inNumSamples);
	void trunc_ai(BinaryOpUGen *unit, int inNumSamples);
	void trunc_ia(BinaryOpUGen *unit, int inNumSamples);
	void atan2_1(BinaryOpUGen *unit, int inNumSamples);
	void atan2_aa(BinaryOpUGen *unit, int inNumSamples);
	void atan2_ak(BinaryOpUGen *unit, int inNumSamples);
	void atan2_ka(BinaryOpUGen *unit, int inNumSamples);
	void atan2_ai(BinaryOpUGen *unit, int inNumSamples);
	void atan2_ia(BinaryOpUGen *unit, int inNumSamples);
	void hypot_1(BinaryOpUGen *unit, int inNumSamples);
	void hypot_aa(BinaryOpUGen *unit, int inNumSamples);
	void hypot_ak(BinaryOpUGen *unit, int inNumSamples);
	void hypot_ka(BinaryOpUGen *unit, int inNumSamples);
	void hypot_ai(BinaryOpUGen *unit, int inNumSamples);
	void hypot_ia(BinaryOpUGen *unit, int inNumSamples);
	void hypotx_1(BinaryOpUGen *unit, int inNumSamples);
	void hypotx_aa(BinaryOpUGen *unit, int inNumSamples);
	void hypotx_ak(BinaryOpUGen *unit, int inNumSamples);
	void hypotx_ka(BinaryOpUGen *unit, int inNumSamples);
	void hypotx_ai(BinaryOpUGen *unit, int inNumSamples);
	void hypotx_ia(BinaryOpUGen *unit, int inNumSamples);
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void ChooseOperatorFunc(BinaryOpUGen *unit);

void BinaryOpUGen_Ctor(BinaryOpUGen *unit)
{	
	ChooseOperatorFunc(unit);
	unit->mPrevA = ZIN0(0);
	unit->mPrevB = ZIN0(1);
	(unit->mCalcFunc)(unit, 1);
}



void zero_1(BinaryOpUGen *unit, int inNumSamples)
{
	ZOUT0(0) = 0.f;
}

void firstarg_1(BinaryOpUGen *unit, int inNumSamples)
{
	ZOUT0(0) = ZIN0(0);
}

void secondarg_1(BinaryOpUGen *unit, int inNumSamples)
{
	ZOUT0(0) = ZIN0(1);
}

void add_1(BinaryOpUGen *unit, int inNumSamples)
{
	ZOUT0(0) = ZIN0(0) + ZIN0(1);
}

void sub_1(BinaryOpUGen *unit, int inNumSamples)
{
	ZOUT0(0) = ZIN0(0) - ZIN0(1);
}

void mul_1(BinaryOpUGen *unit, int inNumSamples)
{
	ZOUT0(0) = ZIN0(0) * ZIN0(1);
}

void div_1(BinaryOpUGen *unit, int inNumSamples)
{
	ZOUT0(0) = ZIN0(0) / ZIN0(1);
}

void mod_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_mod(xa, xb);
}

void max_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_max(xa, xb);
}

void min_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_min(xa, xb);
}

void and_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_andt(xa, xb);	
}

void or_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_ort(xa, xb);	
}

void xor_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_xort(xa, xb);	
}

void amclip_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_amclip(xa, xb);
}

void scaleneg_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa >= 0.f ? xa : xa * xb;
}

void pow_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa >= 0.f ? pow(xa, xb) : -pow(-xa, xb);
}


void ring1_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa * xb + xa;
}

void ring2_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa * xb + xa + xb;
}

void ring3_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa * xa * xb;
}

void ring4_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa * xa * xb - xa * xb * xb;
}

void thresh_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa < xb ? 0.f : xa;
}

void clip2_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa > xb ? xb : (xa < -xb ? -xb : xa);
}

void excess_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa > xb ? xa-xb : (xa < -xb ? xa+xb : 0.f);
}

void lt_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa < xb ? 1.f : 0.f;
}

void le_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa <= xb ? 1.f : 0.f;
}

void gt_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa > xb ? 1.f : 0.f;
}

void ge_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa >= xb ? 1.f : 0.f;
}

void eq_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa == xb ? 1.f : 0.f;
}

void neq_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa != xb ? 1.f : 0.f;
}


void sumsqr_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa * xa + xb * xb;
}

void difsqr_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = xa * xa - xb * xb;
}

void sqrsum_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	float sum = xa + xb;
	ZOUT0(0) = sum * sum;
}

void sqrdif_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	float dif = xa - xb;
	ZOUT0(0) = dif * dif;
}

void absdif_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = fabs(xa - xb);
}

void round_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_round(xa, xb);
}

void trunc_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_trunc(xa, xb);
}

void fold2_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_fold(xa, -xb, xb);
}

void wrap2_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_round(xa, xb);
}

void atan2_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = atan2(xa, xb);
}

void hypot_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = hypot(xa, xb);
}

void hypotx_1(BinaryOpUGen *unit, int inNumSamples)
{
	float xa = ZIN0(0);
	float xb = ZIN0(1);
	ZOUT0(0) = sc_hypotx(xa, xb);
}



void zero_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = OUT(0);
	
	ZClear(inNumSamples, out);
}

void firstarg_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = OUT(0);
	float *a = IN(0);
	
	ZCopy(inNumSamples, out, a);
}

void secondarg_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = OUT(0);
	float *b = IN(1);
	
	ZCopy(inNumSamples, out, b);
}

void add_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = ZXP(a) + ZXP(b);
	);
}

void add_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb == 0.f) {
			ZCopy(inNumSamples, out, a);
		} else {
			float *out = ZOUT(0);
			float *a = ZIN(0);
			LOOP(inNumSamples, 
				ZXP(out) = ZXP(a) + xb;
			);
		}
	} else {
		float slope =  CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			ZXP(out) = ZXP(a) + xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}


void add_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			ZCopy(inNumSamples, out, b);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = xa + ZXP(b);
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			ZXP(out) = xa + ZXP(b);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void add_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = xa + ZXP(b);
	);
	unit->mPrevA = xa;
}


void add_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = ZXP(a) + xb;
	);
	unit->mPrevB = xb;
}



/////////////////////////



void sub_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = ZXP(a) - ZXP(b);
	);
}

void sub_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb == 0.f) {
			ZCopy(inNumSamples, out, a);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = ZXP(a) - xb;
			);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			ZXP(out) = ZXP(a) - xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void sub_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			ZCopy(inNumSamples, out, b);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = xa - ZXP(b);
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			ZXP(out) = xa - ZXP(b);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}


void sub_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = xa - ZXP(b);
	);
	unit->mPrevA = xa;
}


void sub_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = ZXP(a) - xb;
	);
	unit->mPrevB = xb;
}


void mul_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = ZXP(a) * ZXP(b);
	);
}

void mul_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
		
	if (xb == next_b) {
		if (xb == 0.f) {
			ZClear(inNumSamples, out);
		} else if (xb == 1.f) {
			ZCopy(inNumSamples, out, a);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = ZXP(a) * xb;
			);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			ZXP(out) = ZXP(a) * xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void mul_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			ZClear(inNumSamples, out);
		} else if (xa == 1.f) {
			ZCopy(inNumSamples, out, b);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = xa * ZXP(b);
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			ZXP(out) = xa * ZXP(b);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void mul_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = xa * ZXP(b);
	);
	unit->mPrevA = xa;
}


void mul_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = ZXP(a) * xb;
	);
	unit->mPrevB = xb;
}




void div_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = ZXP(a) / ZXP(b);
	);
}

void div_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb == 0.f) {
			ZClear(inNumSamples, out);
		} else if (xb == 1.f) {
			ZCopy(inNumSamples, out, a);
		} else {
			float recip = 1.f / xb;
			LOOP(inNumSamples, 
				ZXP(out) = ZXP(a) * recip;
			);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			ZXP(out) = ZXP(a) / xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void div_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			ZClear(inNumSamples, out);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = xa / ZXP(b);
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			ZXP(out) = xa / ZXP(b);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void div_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = xa / ZXP(b);
	);
	unit->mPrevA = xa;
}


void div_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	float rxb = 1.f / xb;
	LOOP(inNumSamples, 
		ZXP(out) = ZXP(a) * rxb;
	);
	unit->mPrevB = xb;
}




void mod_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_mod(xa, xb);
	);
}

void mod_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb == 0.f) {
			ZCopy(inNumSamples, out, a);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = sc_mod(ZXP(a), xb);
			);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			ZXP(out) = sc_mod(ZXP(a), xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void mod_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			ZClear(inNumSamples, out);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = sc_mod(xa, ZXP(b));
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			ZXP(out) = sc_mod(xa, ZXP(b));
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}


void mod_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = sc_mod(xa, ZXP(b));
	);
	unit->mPrevA = xa;
}


void mod_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		ZXP(out) = sc_mod(ZXP(a), xb);
	);
	unit->mPrevB = xb;
}



void max_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_max(xa, xb);
	);
}

void max_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_max(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_max(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void max_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_max(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_max(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void max_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_max(xa, xb);
	);
	unit->mPrevA = xa;
}


void max_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_max(xa, xb);
	);
	unit->mPrevB = xb;
}



void min_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_min(xa, xb);
	);
}

void min_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_min(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_min(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void min_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_min(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_min(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void min_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_min(xa, xb);
	);
	unit->mPrevA = xa;
}


void min_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_min(xa, xb);
	);
	unit->mPrevB = xb;
}




void and_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_andt(xa, xb) ;
	);
}

void and_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_andt(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_andt(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void and_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_andt(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_andt(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void and_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_andt(xa, xb);
	);
	unit->mPrevA = xa;
}


void and_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_andt(xa, xb);
	);
	unit->mPrevB = xb;
}







void or_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_ort(xa, xb) ;
	);
}

void or_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_ort(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_ort(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void or_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_ort(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_ort(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void or_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_ort(xa, xb);
	);
	unit->mPrevA = xa;
}


void or_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_ort(xa, xb);
	);
	unit->mPrevB = xb;
}







void xor_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_xort(xa, xb) ;
	);
}

void xor_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_xort(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_xort(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void xor_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_ort(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_xort(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void xor_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_xort(xa, xb);
	);
	unit->mPrevA = xa;
}


void xor_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_xort(xa, xb);
	);
	unit->mPrevB = xb;
}



void amclip_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_amclip(xa, xb);
	);
}

void amclip_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb > 0.f) {
			LOOP(inNumSamples, 
				ZXP(out) = ZXP(a) * xb;
			);
		} else {
			ZClear(inNumSamples, out);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_amclip(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void amclip_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_amclip(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_amclip(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void amclip_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_amclip(xa, xb);
	);
	unit->mPrevA = xa;
}


void amclip_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_amclip(xa, xb);
	);
	unit->mPrevB = xb;
}



void scaleneg_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa >= 0.f ? xa : xa * xb;
	);
}

void scaleneg_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa >= 0.f ? xa : xa * xb;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa >= 0.f ? xa : xa * xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void scaleneg_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa >= 0.f) {
			LOOP(inNumSamples, 
				ZXP(out) = xa;
			);
		} else {
			LOOP(inNumSamples, 
				ZXP(out) = xa * ZXP(b);
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa >= 0.f ? xa : xa * xb;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void scaleneg_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa >= 0.f ? xa : xa * xb;
	);
	unit->mPrevA = xa;
}


void scaleneg_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa >= 0.f ? xa : xa * xb;
	);
	unit->mPrevB = xb;
}




void pow_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa >= 0.f ? pow(xa, xb) : -pow(-xa, xb);
	);
}


void pow_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa >= 0.f ? pow(xa, xb) : -pow(-xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa >= 0.f ? pow(xa, xb) : -pow(-xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void pow_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa >= 0.f) {
			LOOP(inNumSamples, 
				float xb = ZXP(b);
				ZXP(out) = pow(xa, xb);
			);
		} else {
			LOOP(inNumSamples, 
				float xb = ZXP(b);
				ZXP(out) = -pow(-xa, xb);
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa >= 0.f ? pow(xa, xb) : -pow(-xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void pow_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa >= 0.f ? pow(xa, xb) : -pow(-xa, xb);
	);
	unit->mPrevA = xa;
}


void pow_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa >= 0.f ? pow(xa, xb) : -pow(-xa, xb);
	);
	unit->mPrevB = xb;
}




void ring1_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa * xb + xa;
	);
}

void ring1_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb == 0.f) {
			ZCopy(inNumSamples, out, a);
		} else if (xb == 1.f) {
			LOOP(inNumSamples, 
				float xa = ZXP(a);
				ZXP(out) = xa + xa;
			);
		} else {
			LOOP(inNumSamples, 
				float xa = ZXP(a);
				ZXP(out) = xa * xb + xa;
			);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa * xb + xa;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void ring1_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			LOOP(inNumSamples, 
				ZXP(out) = 0.f;
			);
		} else {
			LOOP(inNumSamples, 
				float xb = ZXP(b);
				ZXP(out) = xa * xb + xa;
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa * xb + xa;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void ring1_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa * xb + xa;
	);
	unit->mPrevA = xa;
}


void ring1_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa * xb + xa;
	);
	unit->mPrevB = xb;
}




void ring2_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa * xb + xa + xb;
	);
}

void ring2_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb == 0.f) {
			ZCopy(inNumSamples, out, a);
		} else {
			LOOP(inNumSamples, 
				float xa = ZXP(a);
				ZXP(out) = xa * xb + xa + xb;
			);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa * xb + xa + xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void ring2_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			ZCopy(inNumSamples, out, b);
		} else {
			LOOP(inNumSamples, 
				float xb = ZXP(b);
				ZXP(out) = xa * xb + xa + xb;
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa * xb + xa + xb;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void ring2_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa * xb + xa + xb;
	);
	unit->mPrevA = xa;
}


void ring2_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa * xb + xa + xb;
	);
	unit->mPrevB = xb;
}




void ring3_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa * xa * xb;
	);
}

void ring3_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb == 0.f) {
			ZClear(inNumSamples, out);
		} else if (xb == 1.f) {
			LOOP(inNumSamples, 
				float xa = ZXP(a);
				ZXP(out) = xa * xa;
			);
		} else {
			LOOP(inNumSamples, 
				float xa = ZXP(a);
				ZXP(out) = xa * xa * xb;
			);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa * xa * xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void ring3_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			ZClear(inNumSamples, out);
		} else if (xa == 1.f) {
			ZCopy(inNumSamples, out, b);
		} else {
			LOOP(inNumSamples, 
				float xb = ZXP(b);
				ZXP(out) = xa * xa * xb;
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa * xa * xb;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void ring3_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa * xa * xb;
	);
	unit->mPrevA = xa;
}


void ring3_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa * xa * xb;
	);
	unit->mPrevB = xb;
}



void ring4_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa * xa * xb - xa * xb * xb;
	);
}

void ring4_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		if (xb == 0.f) {
			ZClear(inNumSamples, out);
		} else if (xb == 1.f) {
			LOOP(inNumSamples, 
				float xa = ZXP(a);
				ZXP(out) = xa * xa - xa;
			);
		} else {
			LOOP(inNumSamples, 
				float xa = ZXP(a);
				ZXP(out) = xa * xa * xb - xa * xb * xb;
			);
		}
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa * xa * xb - xa * xb * xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void ring4_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		if (xa == 0.f) {
			ZClear(inNumSamples, out);
		} else if (xa == 1.f) {
			LOOP(inNumSamples, 
				float xb = ZXP(b);
				ZXP(out) = xb - xb * xb;
			);
		} else {
			LOOP(inNumSamples, 
				float xb = ZXP(b);
				ZXP(out) = xa * xa * xb - xa * xb * xb;
			);
		}
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa * xa * xb - xa * xb * xb;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void ring4_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa * xa * xb - xa * xb * xb;
	);
	unit->mPrevA = xa;
}


void ring4_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa * xa * xb - xa * xb * xb;
	);
	unit->mPrevB = xb;
}



void thresh_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa < xb ? 0.f : xa;
	);
}

void thresh_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa < xb ? 0.f : xa;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa < xb ? 0.f : xa;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void thresh_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa < xb ? 0.f : xa;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa < xb ? 0.f : xa;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void thresh_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa < xb ? 0.f : xa;
	);
	unit->mPrevA = xa;
}


void thresh_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa < xb ? 0.f : xa;
	);
	unit->mPrevB = xb;
}



void clip2_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa > xb ? xb : (xa < -xb ? -xb : xa);
	);
}

void clip2_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa > xb ? xb : (xa < -xb ? -xb : xa);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa > xb ? xb : (xa < -xb ? -xb : xa);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void clip2_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa > xb ? xb : (xa < -xb ? -xb : xa);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa > xb ? xb : (xa < -xb ? -xb : xa);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void clip2_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa > xb ? xb : (xa < -xb ? -xb : xa);
	);
	unit->mPrevA = xa;
}


void clip2_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa > xb ? xb : (xa < -xb ? -xb : xa);
	);
	unit->mPrevB = xb;
}



void excess_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa > xb ? xa-xb : (xa < -xb ? xa+xb : 0.f);
	);
}

void excess_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa > xb ? xa-xb : (xa < -xb ? xa+xb : 0.f);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa > xb ? xa-xb : (xa < -xb ? xa+xb : 0.f);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void excess_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa > xb ? xa-xb : (xa < -xb ? xa+xb : 0.f);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa > xb ? xa-xb : (xa < -xb ? xa+xb : 0.f);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void excess_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa > xb ? xa-xb : (xa < -xb ? xa+xb : 0.f);
	);
	unit->mPrevA = xa;
}


void excess_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa > xb ? xa-xb : (xa < -xb ? xa+xb : 0.f);
	);
	unit->mPrevB = xb;
}



void lt_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa < xb ? 1.f : 0.f;
	);
}

void lt_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa < xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa < xb ? 1.f : 0.f;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void lt_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa < xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa < xb ? 1.f : 0.f;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void lt_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa < xb ? 1.f : 0.f;
	);
	unit->mPrevA = xa;
}


void lt_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa < xb ? 1.f : 0.f;
	);
	unit->mPrevB = xb;
}



void le_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa <= xb ? 1.f : 0.f;
	);
}

void le_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa <= xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa <= xb ? 1.f : 0.f;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void le_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa <= xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa <= xb ? 1.f : 0.f;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void le_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa <= xb ? 1.f : 0.f;
	);
	unit->mPrevA = xa;
}


void le_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa <= xb ? 1.f : 0.f;
	);
	unit->mPrevB = xb;
}



void gt_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa > xb ? 1.f : 0.f;
	);
}

void gt_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa > xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa > xb ? 1.f : 0.f;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void gt_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa > xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa > xb ? 1.f : 0.f;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void gt_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa > xb ? 1.f : 0.f;
	);
	unit->mPrevA = xa;
}


void gt_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa > xb ? 1.f : 0.f;
	);
	unit->mPrevB = xb;
}



void ge_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa >= xb ? 1.f : 0.f;
	);
}

void ge_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa >= xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa >= xb ? 1.f : 0.f;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void ge_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa >= xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa >= xb ? 1.f : 0.f;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void ge_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa >= xb ? 1.f : 0.f;
	);
	unit->mPrevA = xa;
}


void ge_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa >= xb ? 1.f : 0.f;
	);
	unit->mPrevB = xb;
}



void eq_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa == xb ? 1.f : 0.f;
	);
}

void eq_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa == xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa == xb ? 1.f : 0.f;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void eq_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa == xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa == xb ? 1.f : 0.f;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void eq_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa == xb ? 1.f : 0.f;
	);
	unit->mPrevA = xa;
}


void eq_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa == xb ? 1.f : 0.f;
	);
	unit->mPrevB = xb;
}



void neq_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa != xb ? 1.f : 0.f;
	);
}

void neq_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa != xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa != xb ? 1.f : 0.f;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void neq_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa != xb ? 1.f : 0.f;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa != xb ? 1.f : 0.f;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void neq_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa != xb ? 1.f : 0.f;
	);
	unit->mPrevA = xa;
}


void neq_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa != xb ? 1.f : 0.f;
	);
	unit->mPrevB = xb;
}


void sumsqr_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa * xa + xb * xb;
	);
}

void sumsqr_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa * xa + xb * xb;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa * xa + xb * xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void sumsqr_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa * xa + xb * xb;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa * xa + xb * xb;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}


void sumsqr_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa * xa + xb * xb;
	);
	unit->mPrevA = xa;
}


void sumsqr_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa * xa + xb * xb;
	);
	unit->mPrevB = xb;
}



void difsqr_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = xa * xa - xb * xb;
	);
}

void difsqr_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa * xa - xb * xb;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = xa * xa - xb * xb;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void difsqr_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa * xa - xb * xb;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = xa * xa - xb * xb;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void difsqr_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = xa * xa - xb * xb;
	);
	unit->mPrevA = xa;
}


void difsqr_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = xa * xa - xb * xb;
	);
	unit->mPrevB = xb;
}


void sqrsum_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float sum = ZXP(a) + ZXP(b);
		ZXP(out) = sum * sum;
	);
}

void sqrsum_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			float sum = xa + xb;
			ZXP(out) = sum * sum;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			float sum = xa + xb;
			ZXP(out) = sum * sum;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void sqrsum_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			float sum = xa + xb;
			ZXP(out) = sum * sum;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			float sum = xa + xb;
			ZXP(out) = sum * sum;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void sqrsum_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		float sum = xa + xb;
		ZXP(out) = sum * sum;
	);
	unit->mPrevA = xa;
}


void sqrsum_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float sum = xa + xb;
		ZXP(out) = sum * sum;
	);
	unit->mPrevB = xb;
}


void sqrdif_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float dif = ZXP(a) - ZXP(b);
		ZXP(out) = dif * dif;
	);
}

void sqrdif_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			float dif = xa - xb;
			ZXP(out) = dif * dif;
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			float dif = xa - xb;
			ZXP(out) = dif * dif;
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void sqrdif_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			float dif = xa - xb;
			ZXP(out) = dif * dif;
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			float dif = xa - xb;
			ZXP(out) = dif * dif;
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}


void sqrdif_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		float dif = xa - xb;
		ZXP(out) = dif * dif;
	);
	unit->mPrevA = xa;
}


void sqrdif_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float dif = xa - xb;
		ZXP(out) = dif * dif;
	);
	unit->mPrevB = xb;
}


void absdif_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float dif = ZXP(a) - ZXP(b);
		ZXP(out) = fabs(dif);
	);
}

void absdif_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			float dif = xa - xb;
			ZXP(out) = fabs(dif);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			float dif = xa - xb;
			ZXP(out) = fabs(dif);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void absdif_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			float dif = xa - xb;
			ZXP(out) = fabs(dif);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			float dif = xa - xb;
			ZXP(out) = fabs(dif);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void absdif_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		float dif = xa - xb;
		ZXP(out) = fabs(dif);
	);
	unit->mPrevA = xa;
}


void absdif_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float dif = xa - xb;
		ZXP(out) = fabs(dif);
	);
	unit->mPrevB = xb;
}


void round_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_round(xa, xb);
	);
}

void round_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_round(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_round(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void round_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_round(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_round(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}


void round_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_round(xa, xb);
	);
	unit->mPrevA = xa;
}


void round_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_round(xa, xb);
	);
	unit->mPrevB = xb;
}



void trunc_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_trunc(xa, xb);
	);
}

void trunc_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_trunc(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_trunc(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void trunc_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_trunc(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_trunc(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void trunc_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_trunc(xa, xb);
	);
	unit->mPrevA = xa;
}


void trunc_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_trunc(xa, xb);
	);
	unit->mPrevB = xb;
}



void fold2_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_fold(xa, -xb, xb);
	);
}

void fold2_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_fold(xa, -xb, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_fold(xa, -xb, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void fold2_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_fold(xa, -xb, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_fold(xa, -xb, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void fold2_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_fold(xa, -xb, xb);
	);
	unit->mPrevA = xa;
}


void fold2_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_fold(xa, -xb, xb);
	);
	unit->mPrevB = xb;
}




void wrap2_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_wrap(xa, -xb, xb);
	);
}

void wrap2_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_wrap(xa, -xb, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_wrap(xa, -xb, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void wrap2_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_wrap(xa, -xb, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_wrap(xa, -xb, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void wrap2_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_wrap(xa, -xb, xb);
	);
	unit->mPrevA = xa;
}


void wrap2_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_wrap(xa, -xb, xb);
	);
	unit->mPrevB = xb;
}



void atan2_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = atan2(xa, xb);
	);
}

void atan2_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = atan2(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = atan2(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void atan2_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = atan2(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = atan2(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void atan2_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = atan2(xa, xb);
	);
	unit->mPrevA = xa;
}


void atan2_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = atan2(xa, xb);
	);
	unit->mPrevB = xb;
}


void hypot_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = hypot(xa, xb);
	);
}

void hypot_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = hypot(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = hypot(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void hypot_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = hypot(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = hypot(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void hypot_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = hypot(xa, xb);
	);
	unit->mPrevA = xa;
}


void hypot_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = hypot(xa, xb);
	);
	unit->mPrevB = xb;
}



void hypotx_aa(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		float xb = ZXP(b);
		ZXP(out) = sc_hypotx(xa, xb);
	);
}

void hypotx_ak(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = unit->mPrevB;
	float next_b = ZIN0(1);
	
	if (xb == next_b) {
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_hypotx(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_b, xb);
		LOOP(inNumSamples, 
			float xa = ZXP(a);
			ZXP(out) = sc_hypotx(xa, xb);
			xb += slope;
		);
		unit->mPrevB = xb;
	}
}

void hypotx_ka(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = unit->mPrevA;
	float *b = ZIN(1);
	float next_a = ZIN0(0);
	
	if (xa == next_a) {
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_hypotx(xa, xb);
		);
	} else {
		float slope = CALCSLOPE(next_a, xa);
		LOOP(inNumSamples, 
			float xb = ZXP(b);
			ZXP(out) = sc_hypotx(xa, xb);
			xa += slope;
		);
		unit->mPrevA = xa;
	}
}

void hypotx_ia(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float xa = ZIN0(0);
	float *b = ZIN(1);
	
	LOOP(inNumSamples, 
		float xb = ZXP(b);
		ZXP(out) = sc_hypotx(xa, xb);
	);
	unit->mPrevA = xa;
}


void hypotx_ai(BinaryOpUGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *a = ZIN(0);
	float xb = ZIN0(1);
	
	LOOP(inNumSamples, 
		float xa = ZXP(a);
		ZXP(out) = sc_hypotx(xa, xb);
	);
	unit->mPrevB = xb;
}


#pragma mark


#if __VEC__

void vadd_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_add(vec_ld(i, va), vec_ld(i, vb)), i, vout);
	}
}

void vsub_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_sub(vec_ld(i, va), vec_ld(i, vb)), i, vout);
	}
}

void vmul_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_mul(vec_ld(i, va), vec_ld(i, vb)), i, vout);
	}
}

void vdiv_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_div(vec_ld(i, va), vec_ld(i, vb)), i, vout);
	}
}


void vadd_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_add(zva, vec_ld(i, vb)), i, vout);
	}
}

void vsub_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_sub(zva, vec_ld(i, vb)), i, vout);
	}
}

void vmul_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_mul(zva, vec_ld(i, vb)), i, vout);
	}
}

void vdiv_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_div(zva, vec_ld(i, vb)), i, vout);
	}
}


void vadd_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_add(vec_ld(i, va), zvb), i, vout);
	}
}

void vsub_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_sub(vec_ld(i, va), zvb), i, vout);
	}
}

void vmul_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_mul(vec_ld(i, va), zvb), i, vout);
	}
}

void vdiv_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vec_reciprocal(vload(ZIN0(1)));
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_mul(vec_ld(i, va), zvb), i, vout);
	}
}

void vadd_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;

	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		if (xb == 0.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vec_ld(i, va), i, vout);
			}
		} else {
			vfloat32 vxb = vload(xb);
			for (int i=0; i<len; i += 16) {
				vec_st(vec_add(vec_ld(i, va), vxb), i, vout);
			}
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_add(vec_ld(i, va), vxb), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vsub_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;

	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		if (xb == 0.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vec_ld(i, va), i, vout);
			}
		} else {
			vfloat32 vxb = vload(xb);
			for (int i=0; i<len; i += 16) {
				vec_st(vec_sub(vec_ld(i, va), vxb), i, vout);
			}
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_sub(vec_ld(i, va), vxb), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vmul_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;

	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		if (xb == 0.f) {
			define_vzero;
			for (int i=0; i<len; i += 16) {
				vec_st(vzero, i, vout);
			}
		} else if (xb == 1.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vec_ld(i, va), i, vout);
			}
		} else {
			vfloat32 vxb = vload(xb);
			define_vzero;
			for (int i=0; i<len; i += 16) {
				vec_st(vec_mul(vec_ld(i, va), vxb), i, vout);
			}
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		define_vzero;
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_mul(vec_ld(i, va), vxb), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vdiv_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;

	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		if (xb == 0.f) {
			// don't divide by zero
			define_vzero;
			for (int i=0; i<len; i += 16) {
				vec_st(vzero, i, vout);
			}
		} else if (xb == 1.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vec_ld(i, va), i, vout);
			}
		} else {
			vfloat32 vxb = vec_reciprocal(vload(xb));
			define_vzero;
			for (int i=0; i<len; i += 16) {
				vec_st(vec_mul(vec_ld(i, va), vxb), i, vout);
			}
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		define_vzero;
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_div(vec_ld(i, va), vxb), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}


void vadd_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	
	if (xa == nextA) {
		if (xa == 0.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vec_ld(i, vb), i, vout);
			}
		} else {
			vfloat32 vxa = vload(xa);
			for (int i=0; i<len; i += 16) {
				vec_st(vec_add(vxa, vec_ld(i, vb)), i, vout);
			}
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_add(vxa, vec_ld(i, vb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}


void vsub_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	
	if (xa == nextA) {
		vfloat32 vxa = vload(xa);
		for (int i=0; i<len; i += 16) {
			vec_st(vec_sub(vxa, vec_ld(i, vb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_sub(vxa, vec_ld(i, vb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}


void vmul_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	define_vzero;
	
	if (xa == nextA) {
		if (xa == 0.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vzero, i, vout);
			}
		} else if (xa == 1.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vec_ld(i, vb), i, vout);
			}
		} else {
			vfloat32 vxa = vload(xa);
			for (int i=0; i<len; i += 16) {
				vec_st(vec_mul(vxa, vec_ld(i, vb)), i, vout);
			}
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_mul(vxa, vec_ld(i, vb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}

void vdiv_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	define_vzero;
	
	if (xa == nextA) {
		if (xa == 0.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vzero, i, vout);
			}
		} else if (xa == 1.f) {
			for (int i=0; i<len; i += 16) {
				vec_st(vec_reciprocal(vec_ld(i, vb)), i, vout);
			}
		} else {
			vfloat32 vxa = vload(xa);
			for (int i=0; i<len; i += 16) {
				vec_st(vec_div(vxa, vec_ld(i, vb)), i, vout);
			}
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_div(vxa, vec_ld(i, vb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}



void vmin_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_min(vec_ld(i, va), vec_ld(i, vb)), i, vout);
	}
}

void vmax_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_max(vec_ld(i, va), vec_ld(i, vb)), i, vout);
	}
}


void vmin_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_min(zva, vec_ld(i, vb)), i, vout);
	}
}

void vmax_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_max(zva, vec_ld(i, vb)), i, vout);
	}
}

void vmin_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_min(vec_ld(i, va), zvb), i, vout);
	}
}

void vmax_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_max(vec_ld(i, va), zvb), i, vout);
	}
}

void vmin_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;

	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		vfloat32 vxb = vload(xb);
		for (int i=0; i<len; i += 16) {
			vec_st(vec_min(vec_ld(i, va), vxb), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_min(vec_ld(i, va), vxb), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vmax_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;

	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		vfloat32 vxb = vload(xb);
		for (int i=0; i<len; i += 16) {
			vec_st(vec_max(vec_ld(i, va), vxb), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_max(vec_ld(i, va), vxb), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vmin_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	
	if (xa == nextA) {
		vfloat32 vxa = vload(xa);
		for (int i=0; i<len; i += 16) {
			vec_st(vec_min(vxa, vec_ld(i, vb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_min(vxa, vec_ld(i, vb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}

void vmax_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	
	if (xa == nextA) {
		vfloat32 vxa = vload(xa);
		for (int i=0; i<len; i += 16) {
			vec_st(vec_max(vxa, vec_ld(i, vb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vec_st(vec_max(vxa, vec_ld(i, vb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}



void vlt_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vfloat32 zvb = vec_ld(i, vb);
		vec_st(vec_sel(vzero, vones, vec_cmplt(zva, zvb)), i, vout);
	}
}

void vlt_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vec_st(vec_sel(vzero, vones, vec_cmplt(zva, zvb)), i, vout);
	}
}

void vlt_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zvb = vec_ld(i, vb);
		vec_st(vec_sel(vzero, vones, vec_cmplt(zva, zvb)), i, vout);
	}
}

void vlt_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		vfloat32 vxb = vload(xb);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vzero, vones, vec_cmplt(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vzero, vones, vec_cmplt(vxa, vxb)), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vlt_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	if (xa == nextA) {
		vfloat32 vxa = vload(xa);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vzero, vones, vec_cmplt(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vzero, vones, vec_cmplt(vxa, vxb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}




void vgt_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vfloat32 zvb = vec_ld(i, vb);
		vec_st(vec_sel(vzero, vones, vec_cmpgt(zva, zvb)), i, vout);
	}
}

void vgt_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vec_st(vec_sel(vzero, vones, vec_cmpgt(zva, zvb)), i, vout);
	}
}

void vgt_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zvb = vec_ld(i, vb);
		vec_st(vec_sel(vzero, vones, vec_cmpgt(zva, zvb)), i, vout);
	}
}

void vgt_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		vfloat32 vxb = vload(xb);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vzero, vones, vec_cmpgt(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vzero, vones, vec_cmpgt(vxa, vxb)), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vgt_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	if (xa == nextA) {
		vfloat32 vxa = vload(xa);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vzero, vones, vec_cmpgt(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vzero, vones, vec_cmpgt(vxa, vxb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}




void vle_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vfloat32 zvb = vec_ld(i, vb);
		vec_st(vec_sel(vzero, vones, vec_cmple(zva, zvb)), i, vout);
	}
}

void vle_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vec_st(vec_sel(vzero, vones, vec_cmple(zva, zvb)), i, vout);
	}
}

void vle_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zvb = vec_ld(i, vb);
		vec_st(vec_sel(vzero, vones, vec_cmple(zva, zvb)), i, vout);
	}
}

void vle_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		vfloat32 vxb = vload(xb);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vzero, vones, vec_cmple(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vzero, vones, vec_cmple(vxa, vxb)), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vle_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	if (xa == nextA) {
		vfloat32 vxa = vload(xa);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vzero, vones, vec_cmple(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vzero, vones, vec_cmple(vxa, vxb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}




void vge_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vfloat32 zvb = vec_ld(i, vb);
		vec_st(vec_sel(vzero, vones, vec_cmpge(zva, zvb)), i, vout);
	}
}

void vge_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vec_st(vec_sel(vzero, vones, vec_cmpge(zva, zvb)), i, vout);
	}
}

void vge_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zvb = vec_ld(i, vb);
		vec_st(vec_sel(vzero, vones, vec_cmpge(zva, zvb)), i, vout);
	}
}

void vge_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		vfloat32 vxb = vload(xb);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vzero, vones, vec_cmpge(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vzero, vones, vec_cmpge(vxa, vxb)), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vge_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	define_vzero;
	define_vones;
	
	if (xa == nextA) {
		vfloat32 vxa = vload(xa);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vzero, vones, vec_cmpge(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vzero, vones, vec_cmpge(vxa, vxb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}


void vthresh_aa(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vec_st(vec_sel(zva, vzero, vec_cmplt(zva, vec_ld(i, vb))), i, vout);
	}
}

void vthresh_ai(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	vfloat32 zvb = vload(ZIN0(1));
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vfloat32 zva = vec_ld(i, va);
		vec_st(vec_sel(zva, vzero, vec_cmplt(zva, zvb)), i, vout);
	}
}

void vthresh_ia(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 zva = vload(ZIN0(0));
	vfloat32 *vb = (vfloat32*)IN(1);
	int len = inNumSamples << 2;
	define_vzero;
	
	for (int i=0; i<len; i += 16) {
		vec_st(vec_sel(zva, vzero, vec_cmplt(zva, vec_ld(i, vb))), i, vout);
	}
}

void vthresh_ak(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	vfloat32 *va = (vfloat32*)IN(0);
	float nextB = ZIN0(1);
	int len = inNumSamples << 2;
	define_vzero;
	
	float xb = unit->mPrevB;
	
	if (xb == nextB) {
		vfloat32 vxb = vload(xb);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vxa, vzero, vec_cmplt(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextB, xb);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxb = vstart(xb, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxa = vec_ld(i, va);
			vec_st(vec_sel(vxa, vzero, vec_cmplt(vxa, vxb)), i, vout);
			vxb = vec_add(vxb, vslope);
		}
		unit->mPrevB = nextB;
	}
}

void vthresh_ka(BinaryOpUGen *unit, int inNumSamples)
{
	vfloat32 *vout = (vfloat32*)OUT(0);
	float xa = unit->mPrevA;
	vfloat32 *vb = (vfloat32*)IN(1);
	float nextA = ZIN0(0);
	int len = inNumSamples << 2;
	define_vzero;
	
	if (xa == nextA) {
		vfloat32 vxa = vload(xa);
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vxa, vzero, vec_cmplt(vxa, vxb)), i, vout);
		}
	} else {
		float slope =  CALCSLOPE(nextA, xa);
		vfloat32 vslope = vload(4.f * slope);
		vfloat32 vxa = vstart(xa, vslope);
		
		for (int i=0; i<len; i += 16) {
			vfloat32 vxb = vec_ld(i, vb);
			vec_st(vec_sel(vxa, vzero, vec_cmplt(vxa, vxb)), i, vout);
			vxa = vec_add(vxa, vslope);
		}
		unit->mPrevA = nextA;
	}
}



#endif


#pragma mark



BinaryOpFunc ChooseOneSampleFunc(BinaryOpUGen *unit);
BinaryOpFunc ChooseOneSampleFunc(BinaryOpUGen *unit)
{		
	BinaryOpFunc func = &zero_1;
	
	switch (unit->mSpecialIndex) {
		//case opSilence2 : func = &zero_1; break;
		case opAdd : func = &add_1; break;
		case opSub : func = &sub_1; break;
		case opMul : func = &mul_1; break;
		case opFDiv : func = &div_1; break;
		case opMod : func = &mod_1; break;
		case opEQ  : func = &eq_1; break;
		case opNE  : func = &neq_1; break;
		case opLT  : func = &lt_1; break;
		case opGT  : func = &gt_1; break;
		case opLE  : func = &le_1; break;
		case opGE  : func = &ge_1; break;
		case opMin : func = &min_1; break;
		case opMax : func = &max_1; break;
		case opBitAnd : func = &and_1; break;
		case opBitOr : func = &or_1; break;
		case opBitXor : func = &xor_1; break;
		case opRound : func = &round_1; break;
		case opTrunc : func = &trunc_1; break;
		case opAtan2 : func = &atan2_1; break;
		case opHypot : func = &hypot_1; break;
		case opHypotx : func = &hypotx_1; break;
		case opPow   : func = &pow_1; break;
		case opRing1 : func = &ring1_1; break;
		case opRing2 : func = &ring2_1; break;
		case opRing3 : func = &ring3_1; break;
		case opRing4 : func = &ring4_1; break;
		case opDifSqr : func = &difsqr_1; break;
		case opSumSqr : func = &sumsqr_1; break;
		case opSqrSum : func = &sqrsum_1; break;
		case opSqrDif : func = &sqrdif_1; break;
		case opAbsDif : func = &absdif_1; break;
		case opThresh : func = &thresh_1; break;
		case opAMClip : func = &amclip_1; break;
		case opScaleNeg : func = &scaleneg_1; break;
		case opClip2 : func = &clip2_1; break;
		case opFold2 : func = &fold2_1; break;
		case opWrap2 : func = &wrap2_1; break;
		case opExcess : func = &excess_1; break;
		case opFirstArg : func = &firstarg_1; break;
		//case opSecondArg : func = &secondarg_1; break;
		default : func = &add_aa; break;
	}
	return func;
}

#if __VEC__

BinaryOpFunc ChooseVectorFunc(BinaryOpUGen *unit)
{
	BinaryOpFunc func = &zero_1;

	int rateA = INRATE(0);
	int rateB = INRATE(1);
	
	//printf("ChooseVectorFunc %d %d %d\n", rateA, rateB, unit->mSpecialIndex);
	
	switch (rateA) {
		case calc_FullRate:
			switch (rateB) {
				case calc_FullRate:
					switch (unit->mSpecialIndex) {
						//case opSilence2 : func = &zero_aa; break;
						case opAdd : func = &vadd_aa; break;
						case opSub : func = &vsub_aa; break;
						case opMul : func = &vmul_aa; break;
						case opFDiv : func = &vdiv_aa; break;
						case opMod : func = &mod_aa; break;
						case opEQ  : func = &eq_aa; break;
						case opNE  : func = &neq_aa; break;
						case opLT  : func = &vlt_aa; break;
						case opGT  : func = &vgt_aa; break;
						case opLE  : func = &vle_aa; break;
						case opGE  : func = &vge_aa; break;
						case opMin : func = &vmin_aa; break;
						case opMax : func = &vmax_aa; break;
						case opBitAnd : func = &and_aa; break;
						case opBitOr : func = &or_aa; break;
						case opBitXor : func = &xor_aa; break;
						case opRound : func = &round_aa; break;
						case opTrunc : func = &trunc_aa; break;
						case opAtan2 : func = &atan2_aa; break;
						case opHypot : func = &hypot_aa; break;
						case opHypotx : func = &hypotx_aa; break;
						case opPow   : func = &pow_aa; break;
						case opRing1 : func = &ring1_aa; break;
						case opRing2 : func = &ring2_aa; break;
						case opRing3 : func = &ring3_aa; break;
						case opRing4 : func = &ring4_aa; break;
						case opDifSqr : func = &difsqr_aa; break;
						case opSumSqr : func = &sumsqr_aa; break;
						case opSqrSum : func = &sqrsum_aa; break;
						case opSqrDif : func = &sqrdif_aa; break;
						case opAbsDif : func = &absdif_aa; break;
						case opThresh : func = &vthresh_aa; break;
						case opAMClip : func = &amclip_aa; break;
						case opScaleNeg : func = &scaleneg_aa; break;
						case opClip2 : func = &clip2_aa; break;
						case opFold2 : func = &fold2_aa; break;
						case opWrap2 : func = &wrap2_aa; break;
						case opExcess : func = &excess_aa; break;
						case opFirstArg : func = &firstarg_aa; break;
						//case opSecondArg : func = &secondarg_aa; break;
						default : func = &add_aa; break;
					}
					break;
				case calc_BufRate :
					switch (unit->mSpecialIndex) {
						//case opSilence2 : func = &zero_aa; break;
						case opAdd : func = &vadd_ak; break;
						case opSub : func = &vsub_ak; break;
						case opMul : func = &vmul_ak; break;
						case opFDiv : func = &vdiv_ak; break;
						case opMod : func = &mod_ak; break;
						case opEQ  : func = &eq_ak; break;
						case opNE  : func = &neq_ak; break;
						case opLT  : func = &vlt_ak; break;
						case opGT  : func = &vgt_ak; break;
						case opLE  : func = &vle_ak; break;
						case opGE  : func = &vge_ak; break;
						case opMin : func = &vmin_ak; break;
						case opMax : func = &vmax_ak; break;
						case opBitAnd : func = &and_ak; break;
						case opBitOr : func = &or_ak; break;
						case opBitXor : func = &xor_ak; break;
						case opRound : func = &round_ak; break;
						case opTrunc : func = &trunc_ak; break;
						case opAtan2 : func = &atan2_ak; break;
						case opHypot : func = &hypot_ak; break;
						case opHypotx : func = &hypotx_ak; break;
						case opPow   : func = &pow_ak; break;
						case opRing1 : func = &ring1_ak; break;
						case opRing2 : func = &ring2_ak; break;
						case opRing3 : func = &ring3_ak; break;
						case opRing4 : func = &ring4_ak; break;
						case opDifSqr : func = &difsqr_ak; break;
						case opSumSqr : func = &sumsqr_ak; break;
						case opSqrSum : func = &sqrsum_ak; break;
						case opSqrDif : func = &sqrdif_ak; break;
						case opAbsDif : func = &absdif_ak; break;
						case opThresh : func = &vthresh_ak; break;
						case opAMClip : func = &amclip_ak; break;
						case opScaleNeg : func = &scaleneg_ak; break;
						case opClip2 : func = &clip2_ak; break;
						case opFold2 : func = &fold2_ak; break;
						case opWrap2 : func = &wrap2_ak; break;
						case opExcess : func = &excess_ak; break;
						case opFirstArg : func = &firstarg_aa; break;
						//case opSecondArg : func = &secondarg_aa; break;
						default : func = &add_ak; break;
					}
					break;
				case calc_ScalarRate :
					switch (unit->mSpecialIndex) {
						//case opSilence2 : func = &zero_aa; break;
						case opAdd : func = &vadd_ai; break;
						case opSub : func = &vsub_ai; break;
						case opMul : func = &vmul_ai; break;
						case opFDiv : func = &vdiv_ai; break;
						case opMod : func = &mod_ai; break;
						case opEQ  : func = &eq_ai; break;
						case opNE  : func = &neq_ai; break;
						case opLT  : func = &vlt_ai; break;
						case opGT  : func = &vgt_ai; break;
						case opLE  : func = &vle_ai; break;
						case opGE  : func = &vge_ai; break;
						case opMin : func = &vmin_ai; break;
						case opMax : func = &vmax_ai; break;
						case opBitAnd : func = &and_ai; break;
						case opBitOr : func = &or_ai; break;
						case opBitXor : func = &xor_ai; break;
						case opRound : func = &round_ai; break;
						case opTrunc : func = &trunc_ai; break;
						case opAtan2 : func = &atan2_ai; break;
						case opHypot : func = &hypot_ai; break;
						case opHypotx : func = &hypotx_ai; break;
						case opPow   : func = &pow_ai; break;
						case opRing1 : func = &ring1_ai; break;
						case opRing2 : func = &ring2_ai; break;
						case opRing3 : func = &ring3_ai; break;
						case opRing4 : func = &ring4_ai; break;
						case opDifSqr : func = &difsqr_ai; break;
						case opSumSqr : func = &sumsqr_ai; break;
						case opSqrSum : func = &sqrsum_ai; break;
						case opSqrDif : func = &sqrdif_ai; break;
						case opAbsDif : func = &absdif_ai; break;
						case opThresh : func = &vthresh_ai; break;
						case opAMClip : func = &amclip_ai; break;
						case opScaleNeg : func = &scaleneg_ai; break;
						case opClip2 : func = &clip2_ai; break;
						case opFold2 : func = &fold2_ai; break;
						case opWrap2 : func = &wrap2_ai; break;
						case opExcess : func = &excess_ai; break;
						case opFirstArg : func = &firstarg_aa; break;
						//case opSecondArg : func = &secondarg_aa; break;
						default : func = &add_ai; break;
					}
				}
			break;
		case calc_BufRate :
			if (rateB == calc_FullRate) {
				switch (unit->mSpecialIndex) {
					//case opSilence2 : func = &zero_aa; break;
					case opAdd : func = &vadd_ka; break;
					case opSub : func = &vsub_ka; break;
					case opMul : func = &vmul_ka; break;
					case opFDiv : func = &vdiv_ka; break;
					case opMod : func = &mod_ka; break;
					case opEQ  : func = &eq_ka; break;
					case opNE  : func = &neq_ka; break;
					case opLT  : func = &vlt_ka; break;
					case opGT  : func = &vgt_ka; break;
					case opLE  : func = &vle_ka; break;
					case opGE  : func = &vge_ka; break;
					case opMin : func = &vmin_ka; break;
					case opMax : func = &vmax_ka; break;
					case opBitAnd : func = &and_ka; break;
					case opBitOr : func = &or_ka; break;
					case opBitXor : func = &xor_ka; break;
					case opRound : func = &round_ka; break;
					case opTrunc : func = &trunc_ka; break;
					case opAtan2 : func = &atan2_ka; break;
					case opHypot : func = &hypot_ka; break;
					case opHypotx : func = &hypotx_ka; break;
					case opPow   : func = &pow_ka; break;
					case opRing1 : func = &ring1_ka; break;
					case opRing2 : func = &ring2_ka; break;
					case opRing3 : func = &ring3_ka; break;
					case opRing4 : func = &ring4_ka; break;
					case opDifSqr : func = &difsqr_ka; break;
					case opSumSqr : func = &sumsqr_ka; break;
					case opSqrSum : func = &sqrsum_ka; break;
					case opSqrDif : func = &sqrdif_ka; break;
					case opAbsDif : func = &absdif_ka; break;
					case opThresh : func = &vthresh_ka; break;
					case opAMClip : func = &amclip_ka; break;
					case opScaleNeg : func = &scaleneg_ka; break;
					case opClip2 : func = &clip2_ka; break;
					case opFold2 : func = &fold2_ka; break;
					case opWrap2 : func = &wrap2_ka; break;
					case opExcess : func = &excess_ka; break;
					//case opFirstArg : func = &firstarg_aa; break;
					//case opSecondArg : func = &secondarg_aa; break;
					default : func = &add_ka; break;
				}
			} else {
				// this should have been caught by mBufLength == 1
				func = &zero_aa;
			}
			break;
		case calc_ScalarRate :
			if (rateB == calc_FullRate) {
				switch (unit->mSpecialIndex) {
					//case opSilence2 : func = &zero_aa; break;
					case opAdd : func = &vadd_ia; break;
					case opSub : func = &vsub_ia; break;
					case opMul : func = &vmul_ia; break;
					case opFDiv : func = &vdiv_ia; break;
					case opMod : func = &mod_ia; break;
					case opEQ  : func = &eq_ia; break;
					case opNE  : func = &neq_ia; break;
					case opLT  : func = &vlt_ia; break;
					case opGT  : func = &vgt_ia; break;
					case opLE  : func = &vle_ia; break;
					case opGE  : func = &vge_ia; break;
					case opMin : func = &vmin_ia; break;
					case opMax : func = &vmax_ia; break;
					case opBitAnd : func = &and_ia; break;
					case opBitOr : func = &or_ia; break;
					case opBitXor : func = &xor_ia; break;
					case opRound : func = &round_ia; break;
					case opTrunc : func = &trunc_ia; break;
					case opAtan2 : func = &atan2_ia; break;
					case opHypot : func = &hypot_ia; break;
					case opHypotx : func = &hypotx_ia; break;
					case opPow   : func = &pow_ia; break;
					case opRing1 : func = &ring1_ia; break;
					case opRing2 : func = &ring2_ia; break;
					case opRing3 : func = &ring3_ia; break;
					case opRing4 : func = &ring4_ia; break;
					case opDifSqr : func = &difsqr_ia; break;
					case opSumSqr : func = &sumsqr_ia; break;
					case opSqrSum : func = &sqrsum_ia; break;
					case opSqrDif : func = &sqrdif_ia; break;
					case opAbsDif : func = &absdif_ia; break;
					case opThresh : func = &vthresh_ia; break;
					case opAMClip : func = &amclip_ia; break;
					case opScaleNeg : func = &scaleneg_ia; break;
					case opClip2 : func = &clip2_ia; break;
					case opFold2 : func = &fold2_ia; break;
					case opWrap2 : func = &wrap2_ia; break;
					case opExcess : func = &excess_ia; break;
					//case opFirstArg : func = &firstarg_aa; break;
					//case opSecondArg : func = &secondarg_aa; break;
					default : func = &add_ia; break;
				}
			} else {
				// this should have been caught by mBufLength == 1
				func = &zero_aa;
			}
			break;
	}

	return func;
}

#endif

BinaryOpFunc ChooseNormalFunc(BinaryOpUGen *unit)
{
	BinaryOpFunc func = &zero_1;

	int rateA = INRATE(0);
	int rateB = INRATE(1);
	
	switch (rateA) {
		case calc_FullRate:
			switch (rateB) {
				case calc_FullRate:
					switch (unit->mSpecialIndex) {
						//case opSilence2 : func = &zero_aa; break;
						case opAdd : func = &add_aa; break;
						case opSub : func = &sub_aa; break;
						case opMul : func = &mul_aa; break;
						case opFDiv : func = &div_aa; break;
						case opMod : func = &mod_aa; break;
						case opEQ  : func = &eq_aa; break;
						case opNE  : func = &neq_aa; break;
						case opLT  : func = &lt_aa; break;
						case opGT  : func = &gt_aa; break;
						case opLE  : func = &le_aa; break;
						case opGE  : func = &ge_aa; break;
						case opMin : func = &min_aa; break;
						case opMax : func = &max_aa; break;
						case opBitAnd : func = &and_aa; break;
						case opBitOr : func = &or_aa; break;
						case opBitXor : func = &xor_aa; break;
						case opRound : func = &round_aa; break;
						case opTrunc : func = &trunc_aa; break;
						case opAtan2 : func = &atan2_aa; break;
						case opHypot : func = &hypot_aa; break;
						case opHypotx : func = &hypotx_aa; break;
						case opPow   : func = &pow_aa; break;
						case opRing1 : func = &ring1_aa; break;
						case opRing2 : func = &ring2_aa; break;
						case opRing3 : func = &ring3_aa; break;
						case opRing4 : func = &ring4_aa; break;
						case opDifSqr : func = &difsqr_aa; break;
						case opSumSqr : func = &sumsqr_aa; break;
						case opSqrSum : func = &sqrsum_aa; break;
						case opSqrDif : func = &sqrdif_aa; break;
						case opAbsDif : func = &absdif_aa; break;
						case opThresh : func = &thresh_aa; break;
						case opAMClip : func = &amclip_aa; break;
						case opScaleNeg : func = &scaleneg_aa; break;
						case opClip2 : func = &clip2_aa; break;
						case opFold2 : func = &fold2_aa; break;
						case opWrap2 : func = &wrap2_aa; break;
						case opExcess : func = &excess_aa; break;
						case opFirstArg : func = &firstarg_aa; break;
						//case opSecondArg : func = &secondarg_aa; break;
						default : func = &add_aa; break;
					}
					break;
				case calc_BufRate :
					switch (unit->mSpecialIndex) {
						//case opSilence2 : func = &zero_aa; break;
						case opAdd : func = &add_ak; break;
						case opSub : func = &sub_ak; break;
						case opMul : func = &mul_ak; break;
						case opFDiv : func = &div_ak; break;
						case opMod : func = &mod_ak; break;
						case opEQ  : func = &eq_ak; break;
						case opNE  : func = &neq_ak; break;
						case opLT  : func = &lt_ak; break;
						case opGT  : func = &gt_ak; break;
						case opLE  : func = &le_ak; break;
						case opGE  : func = &ge_ak; break;
						case opMin : func = &min_ak; break;
						case opMax : func = &max_ak; break;
						case opBitAnd : func = &and_ak; break;
						case opBitOr : func = &or_ak; break;
						case opBitXor : func = &xor_ak; break;
						case opRound : func = &round_ak; break;
						case opTrunc : func = &trunc_ak; break;
						case opAtan2 : func = &atan2_ak; break;
						case opHypot : func = &hypot_ak; break;
						case opHypotx : func = &hypotx_ak; break;
						case opPow   : func = &pow_ak; break;
						case opRing1 : func = &ring1_ak; break;
						case opRing2 : func = &ring2_ak; break;
						case opRing3 : func = &ring3_ak; break;
						case opRing4 : func = &ring4_ak; break;
						case opDifSqr : func = &difsqr_ak; break;
						case opSumSqr : func = &sumsqr_ak; break;
						case opSqrSum : func = &sqrsum_ak; break;
						case opSqrDif : func = &sqrdif_ak; break;
						case opAbsDif : func = &absdif_ak; break;
						case opThresh : func = &thresh_ak; break;
						case opAMClip : func = &amclip_ak; break;
						case opScaleNeg : func = &scaleneg_ak; break;
						case opClip2 : func = &clip2_ak; break;
						case opFold2 : func = &fold2_ak; break;
						case opWrap2 : func = &wrap2_ak; break;
						case opExcess : func = &excess_ak; break;
						case opFirstArg : func = &firstarg_aa; break;
						//case opSecondArg : func = &secondarg_aa; break;
						default : func = &add_ak; break;
					}
					break;
				case calc_ScalarRate :
					switch (unit->mSpecialIndex) {
						//case opSilence2 : func = &zero_aa; break;
						case opAdd : func = &add_ai; break;
						case opSub : func = &sub_ai; break;
						case opMul : func = &mul_ai; break;
						case opFDiv : func = &div_ai; break;
						case opMod : func = &mod_ai; break;
						case opEQ  : func = &eq_ai; break;
						case opNE  : func = &neq_ai; break;
						case opLT  : func = &lt_ai; break;
						case opGT  : func = &gt_ai; break;
						case opLE  : func = &le_ai; break;
						case opGE  : func = &ge_ai; break;
						case opMin : func = &min_ai; break;
						case opMax : func = &max_ai; break;
						case opBitAnd : func = &and_ai; break;
						case opBitOr : func = &or_ai; break;
						case opBitXor : func = &xor_ai; break;
						case opRound : func = &round_ai; break;
						case opTrunc : func = &trunc_ai; break;
						case opAtan2 : func = &atan2_ai; break;
						case opHypot : func = &hypot_ai; break;
						case opHypotx : func = &hypotx_ai; break;
						case opPow   : func = &pow_ai; break;
						case opRing1 : func = &ring1_ai; break;
						case opRing2 : func = &ring2_ai; break;
						case opRing3 : func = &ring3_ai; break;
						case opRing4 : func = &ring4_ai; break;
						case opDifSqr : func = &difsqr_ai; break;
						case opSumSqr : func = &sumsqr_ai; break;
						case opSqrSum : func = &sqrsum_ai; break;
						case opSqrDif : func = &sqrdif_ai; break;
						case opAbsDif : func = &absdif_ai; break;
						case opThresh : func = &thresh_ai; break;
						case opAMClip : func = &amclip_ai; break;
						case opScaleNeg : func = &scaleneg_ai; break;
						case opClip2 : func = &clip2_ai; break;
						case opFold2 : func = &fold2_ai; break;
						case opWrap2 : func = &wrap2_ai; break;
						case opExcess : func = &excess_ai; break;
						case opFirstArg : func = &firstarg_aa; break;
						//case opSecondArg : func = &secondarg_aa; break;
						default : func = &add_ai; break;
					}
				}
			break;
		case calc_BufRate :
			if (rateB == calc_FullRate) {
				switch (unit->mSpecialIndex) {
					//case opSilence2 : func = &zero_aa; break;
					case opAdd : func = &add_ka; break;
					case opSub : func = &sub_ka; break;
					case opMul : func = &mul_ka; break;
					case opFDiv : func = &div_ka; break;
					case opMod : func = &mod_ka; break;
					case opEQ  : func = &eq_ka; break;
					case opNE  : func = &neq_ka; break;
					case opLT  : func = &lt_ka; break;
					case opGT  : func = &gt_ka; break;
					case opLE  : func = &le_ka; break;
					case opGE  : func = &ge_ka; break;
					case opMin : func = &min_ka; break;
					case opMax : func = &max_ka; break;
					case opBitAnd : func = &and_ka; break;
					case opBitOr : func = &or_ka; break;
					case opBitXor : func = &xor_ka; break;
					case opRound : func = &round_ka; break;
					case opTrunc : func = &trunc_ka; break;
					case opAtan2 : func = &atan2_ka; break;
					case opHypot : func = &hypot_ka; break;
					case opHypotx : func = &hypotx_ka; break;
					case opPow   : func = &pow_ka; break;
					case opRing1 : func = &ring1_ka; break;
					case opRing2 : func = &ring2_ka; break;
					case opRing3 : func = &ring3_ka; break;
					case opRing4 : func = &ring4_ka; break;
					case opDifSqr : func = &difsqr_ka; break;
					case opSumSqr : func = &sumsqr_ka; break;
					case opSqrSum : func = &sqrsum_ka; break;
					case opSqrDif : func = &sqrdif_ka; break;
					case opAbsDif : func = &absdif_ka; break;
					case opThresh : func = &thresh_ka; break;
					case opAMClip : func = &amclip_ka; break;
					case opScaleNeg : func = &scaleneg_ka; break;
					case opClip2 : func = &clip2_ka; break;
					case opFold2 : func = &fold2_ka; break;
					case opWrap2 : func = &wrap2_ka; break;
					case opExcess : func = &excess_ka; break;
					//case opFirstArg : func = &firstarg_aa; break;
					//case opSecondArg : func = &secondarg_aa; break;
					default : func = &add_ka; break;
				}
			} else {
				// this should have been caught by mBufLength == 1
				func = &zero_aa;
			}
			break;
		case calc_ScalarRate :
			if (rateB == calc_FullRate) {
				switch (unit->mSpecialIndex) {
					//case opSilence2 : func = &zero_aa; break;
					case opAdd : func = &add_ia; break;
					case opSub : func = &sub_ia; break;
					case opMul : func = &mul_ia; break;
					case opFDiv : func = &div_ia; break;
					case opMod : func = &mod_ia; break;
					case opEQ  : func = &eq_ia; break;
					case opNE  : func = &neq_ia; break;
					case opLT  : func = &lt_ia; break;
					case opGT  : func = &gt_ia; break;
					case opLE  : func = &le_ia; break;
					case opGE  : func = &ge_ia; break;
					case opMin : func = &min_ia; break;
					case opMax : func = &max_ia; break;
					case opBitAnd : func = &and_ia; break;
					case opBitOr : func = &or_ia; break;
					case opBitXor : func = &xor_ia; break;
					case opRound : func = &round_ia; break;
					case opTrunc : func = &trunc_ia; break;
					case opAtan2 : func = &atan2_ia; break;
					case opHypot : func = &hypot_ia; break;
					case opHypotx : func = &hypotx_ia; break;
					case opPow   : func = &pow_ia; break;
					case opRing1 : func = &ring1_ia; break;
					case opRing2 : func = &ring2_ia; break;
					case opRing3 : func = &ring3_ia; break;
					case opRing4 : func = &ring4_ia; break;
					case opDifSqr : func = &difsqr_ia; break;
					case opSumSqr : func = &sumsqr_ia; break;
					case opSqrSum : func = &sqrsum_ia; break;
					case opSqrDif : func = &sqrdif_ia; break;
					case opAbsDif : func = &absdif_ia; break;
					case opThresh : func = &thresh_ia; break;
					case opAMClip : func = &amclip_ia; break;
					case opScaleNeg : func = &scaleneg_ia; break;
					case opClip2 : func = &clip2_ia; break;
					case opFold2 : func = &fold2_ia; break;
					case opWrap2 : func = &wrap2_ia; break;
					case opExcess : func = &excess_ia; break;
					//case opFirstArg : func = &firstarg_aa; break;
					//case opSecondArg : func = &secondarg_aa; break;
					default : func = &add_ia; break;
				}
			} else {
				// this should have been caught by mBufLength == 1
				func = &zero_aa;
			}
			break;
	}

	return func;
}


void ChooseOperatorFunc(BinaryOpUGen *unit)
{
	//Print("->ChooseOperatorFunc %d\n", unit->mSpecialIndex);
	BinaryOpFunc func = &zero_aa;
	
	if (BUFLENGTH == 1) {
		func = ChooseOneSampleFunc(unit);
#if __VEC__
	} else if (USEVEC) {
		func = ChooseVectorFunc(unit);
#endif
	} else {
		func = ChooseNormalFunc(unit);
	}
	unit->mCalcFunc = (UnitCalcFunc)func;
	//Print("<-ChooseOperatorFunc %08X\n", func);
	//Print("calc %d\n", unit->mCalcRate);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////

void load(InterfaceTable *inTable)
{
	ft = inTable;

	DefineSimpleUnit(BinaryOpUGen);
}
