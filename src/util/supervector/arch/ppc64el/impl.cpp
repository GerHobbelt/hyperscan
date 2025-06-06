/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2020-2021, VectorCamp PC
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SIMD_IMPL_HPP
#define SIMD_IMPL_HPP

#include <cstdint>
#include <cstdio>

#include "ue2common.h"
#include "util/arch.h"
#include "util/unaligned.h"
#include "util/supervector/supervector.hpp"
#include <iostream>

// 128-bit Powerpc64le implementation

template<>
really_inline SuperVector<16>::SuperVector(SuperVector const &other)
{
    u.v128[0] = other.u.v128[0];
}

template<>
really_inline SuperVector<16>::SuperVector(typename base_type::type const v)
{
    u.v128[0] = v;
};

template<>
template<>
really_inline SuperVector<16>::SuperVector(int8_t const other)
{
    u.v128[0] = (m128) vec_splats(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint8_t const other)
{
    u.v128[0] = (m128) vec_splats(static_cast<uint8_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int16_t const other)
{
    u.v128[0] = (m128) vec_splats(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint16_t const other)
{
    u.v128[0] = (m128) vec_splats(static_cast<uint16_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int32_t const other)
{
    u.v128[0] = (m128) vec_splats(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint32_t const other)
{
    u.v128[0] = (m128) vec_splats(static_cast<uint32_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int64_t const other)
{
    u.v128[0] = (m128) vec_splats(static_cast<ulong64_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint64_t const other)
{
    u.v128[0] = (m128) vec_splats(static_cast<ulong64_t>(other));
}

// Constants
template<>
really_inline SuperVector<16> SuperVector<16>::Ones(void)
{
    return  {(m128) vec_splat_s8(-1)};
}

template<>
really_inline SuperVector<16> SuperVector<16>::Zeroes(void)
{
    return  {(m128) vec_splat_s8(0)};
}

// Methods

template <>
really_inline void SuperVector<16>::operator=(SuperVector<16> const &other)
{
    u.v128[0] = other.u.v128[0];
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator&(SuperVector<16> const &b) const
{
    return {vec_and(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator|(SuperVector<16> const &b) const
{
    return  {vec_or(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator^(SuperVector<16> const &b) const
{
    return  {(m128) vec_xor(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator!() const
{
    return  {(m128) vec_xor(u.v128[0], u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::opandnot(SuperVector<16> const &b) const
{
   m128 not_res = vec_xor(u.v128[0], (m128)vec_splat_s8(-1));
   return {(m128) vec_and(not_res, (m128)b.u.v128[0]) };
}


template <>
really_inline SuperVector<16> SuperVector<16>::operator==(SuperVector<16> const &b) const
{
    return {(m128) vec_cmpeq(u.s8x16[0], b.u.s8x16[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator!=(SuperVector<16> const &b) const
{
    return !(*this == b);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>(SuperVector<16> const &b) const
{ 
    return {(m128) vec_cmpgt(u.v128[0], b.u.v128[0])}; 
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>=(SuperVector<16> const &b) const
{
    return {(m128) vec_cmpge(u.v128[0], b.u.v128[0])};  
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<(SuperVector<16> const &b) const
{
    return {(m128) vec_cmpgt(b.u.v128[0], u.v128[0])};  
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<=(SuperVector<16> const &b) const
{   
    return {(m128) vec_cmpge(b.u.v128[0], u.v128[0])};   
}


template <>
really_inline SuperVector<16> SuperVector<16>::eq(SuperVector<16> const &b) const
{
    return (*this == b);
}

template <>
really_inline typename SuperVector<16>::movemask_type SuperVector<16>::movemask(void)const
{ 
    uint8x16_t s1 = vec_sr((uint8x16_t)u.v128[0], vec_splat_u8(7));
    
    uint16x8_t ss = vec_sr((uint16x8_t)s1, vec_splat_u16(7));
    uint16x8_t res_and = vec_and((uint16x8_t)s1, vec_splats((uint16_t)0xff));
    uint16x8_t s2 = vec_or((uint16x8_t)ss, res_and);
    
    uint32x4_t ss2 = vec_sr((uint32x4_t)s2 , vec_splat_u32(14));
    uint32x4_t res_and2 = vec_and((uint32x4_t)s2, vec_splats((uint32_t)0xff));
    uint32x4_t s3 = vec_or((uint32x4_t)ss2, res_and2);

    uint64x2_t ss3 = vec_sr((uint64x2_t)s3, (uint64x2_t)vec_splats(28));
    uint64x2_t res_and3 = vec_and((uint64x2_t)s3, vec_splats((ulong64_t)0xff));
    uint64x2_t s4 = vec_or((uint64x2_t)ss3, res_and3);

    uint64x2_t ss4 = vec_sld((uint64x2_t) vec_splats(0), s4, 9);
    uint64x2_t res_and4 = vec_and((uint64x2_t)s4, vec_splats((ulong64_t)0xff));
    uint64x2_t s5 = vec_or((uint64x2_t)ss4, res_and4);
    
    return s5[0];
}

template <>
really_inline typename SuperVector<16>::movemask_type SuperVector<16>::eqmask(SuperVector<16> const b) const
{
    return eq(b).movemask();  
}


template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_8_imm() const
{
    return { (m128) vec_sl(u.s8x16[0], vec_splats((uint8_t)N)) }; 
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_16_imm() const
{
    return { (m128) vec_sl(u.s16x8[0], vec_splats((uint16_t)N)) };
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_32_imm() const
{
    return { (m128) vec_sl(u.s32x4[0], vec_splats((uint32_t)N)) };
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_64_imm() const
{
    return { (m128) vec_sl(u.s64x2[0], vec_splats((ulong64_t)N)) };
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_128_imm() const
{
    return { (m128) vec_sld(u.s8x16[0], (int8x16_t)vec_splat_s8(0), N)}; 
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_imm() const
{
   return vshl_128_imm<N>();
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_8_imm() const
{
    return { (m128) vec_sr(u.s8x16[0], vec_splats((uint8_t)N)) };
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_16_imm() const
{
    return { (m128) vec_sr(u.s16x8[0], vec_splats((uint16_t)N)) }; 
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_32_imm() const
{
    return { (m128) vec_sr(u.s32x4[0], vec_splats((uint32_t)N)) };
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_64_imm() const
{		 
   return { (m128) vec_sr(u.s64x2[0], vec_splats((ulong64_t)N)) }; 
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_128_imm() const
{   
    return { (m128) vec_sld((int8x16_t)vec_splat_s8(0), u.s8x16[0], 16 - N) };	
}

template <>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_imm() const
{
    return vshr_128_imm<N>();
}

#if !defined(HS_OPTIMIZE)
template SuperVector<16> SuperVector<16>::vshl_8_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshl_16_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_64_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_64_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshl_128_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_128_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_8_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_8_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_16_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_64_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_64_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_128_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_128_imm<4>() const;
#endif

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_8  (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sl(v->u.s8x16[0], vec_splats((uint8_t)n))}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_16 (uint8_t const UNUSED N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result; 
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sl(v->u.s16x8[0], vec_splats((uint16_t)n))}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_32 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sl(v->u.s32x4[0], vec_splats((uint32_t)n))}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_64 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sl(v->u.s64x2[0], vec_splats((ulong64_t)n))}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_128(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sld(v->u.s8x16[0], (int8x16_t)vec_splat_s8(0), n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl(uint8_t const N) const
{
    return vshl_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_8  (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sr(v->u.s8x16[0], vec_splats((uint8_t)n))}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_16 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sr(v->u.s16x8[0], vec_splats((uint16_t)n))}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_32 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sr(v->u.s32x4[0], vec_splats((uint32_t)n))}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_64 (uint8_t const N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sr(v->u.s64x2[0], vec_splats((ulong64_t)n))}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_128(uint8_t const UNUSED N) const
{
    if (N == 0) return *this;
    if (N == 16) return Zeroes();
    SuperVector result;
    Unroller<1, 16>::iterator([&,v=this](auto const i) { constexpr uint8_t n = i.value; if (N == n) result = {(m128) vec_sld((int8x16_t)vec_splat_u8(0), v->u.s8x16[0], 16 - n)}; });
    return result;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr(uint8_t const N) const
{
    return vshr_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    switch(N) {
    case 1: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0], 15)}; break;
    case 2: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0], 14)}; break;
    case 3: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0], 13)}; break;
    case 4: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0], 12)}; break;
    case 5: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0], 11)}; break;
    case 6: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0], 10)}; break;
    case 7: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0],  9)}; break;
    case 8: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0],  8)}; break;
    case 9: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0),  u.s8x16[0],  7)}; break;
    case 10: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0), u.s8x16[0], 6)}; break;
    case 11: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0), u.s8x16[0], 5)}; break;
    case 12: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0), u.s8x16[0], 4)}; break;
    case 13: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0), u.s8x16[0], 3)}; break;
    case 14: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0), u.s8x16[0], 2)}; break;
    case 15: return {(m128) vec_sld((int8x16_t) vec_splat_s8(0), u.s8x16[0], 1)}; break;
    case 16: return Zeroes(); break;
    default: break;
    }
    return *this;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    switch(N) {
    case 1: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 1)}; break;
    case 2: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 2)}; break;
    case 3: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 3)}; break;
    case 4: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 4)}; break;
    case 5: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 5)}; break;
    case 6: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 6)}; break;
    case 7: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 7)}; break;
    case 8: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 8)}; break;
    case 9: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 9)}; break;
    case 10: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 10)}; break;
    case 11: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 11)}; break;
    case 12: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 12)}; break;
    case 13: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 13)}; break;
    case 14: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 14)}; break;
    case 15: return {(m128) vec_sld(u.s8x16[0], (int8x16_t) vec_splat_s8(0), 15)}; break;
    case 16: return Zeroes(); break;
    default: break;
    }
    return *this;
}

template<>
really_inline SuperVector<16> SuperVector<16>::Ones_vshr(uint8_t const N)
{
    return Ones().vshr_128(N);
}

template<>
really_inline SuperVector<16> SuperVector<16>::Ones_vshl(uint8_t const N)
{
    return Ones().vshl_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu(void const *ptr)
{
    return (m128) vec_xl(0, (const long64_t*)ptr);
}

template <>
really_inline SuperVector<16> SuperVector<16>::load(void const *ptr)
{
    assert(ISALIGNED_N(ptr, alignof(SuperVector::size)));
    return (m128)  vec_xl(0, (const long64_t*)ptr);
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu_maskz(void const *ptr, uint8_t const len)
{
    SuperVector<16> mask = Ones_vshr(16 -len);
    mask.print8("mask");
    SuperVector<16> v = loadu(ptr);
    v.print8("v");
    return mask & v;
}

template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{   
    
    switch(offset) {
    case 0: return other; break;
    case 1: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0], 15)}; break;
    case 2: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0], 14)}; break;
    case 3: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0], 13)}; break;
    case 4: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0], 12)}; break;
    case 5: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0], 11)}; break;
    case 6: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0], 10)}; break;
    case 7: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0],  9)}; break;
    case 8: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0],  8)}; break;
    case 9: return {(m128) vec_sld(u.s8x16[0],  other.u.s8x16[0],  7)}; break;
    case 10: return {(m128) vec_sld(u.s8x16[0], other.u.s8x16[0], 6)}; break;
    case 11: return {(m128) vec_sld(u.s8x16[0], other.u.s8x16[0], 5)}; break;
    case 12: return {(m128) vec_sld(u.s8x16[0], other.u.s8x16[0], 4)}; break;
    case 13: return {(m128) vec_sld(u.s8x16[0], other.u.s8x16[0], 3)}; break;
    case 14: return {(m128) vec_sld(u.s8x16[0], other.u.s8x16[0], 2)}; break;
    case 15: return {(m128) vec_sld(u.s8x16[0], other.u.s8x16[0], 1)}; break;
    default: break;
    }
    return *this;
}

template<>
template<>
really_inline SuperVector<16> SuperVector<16>::pshufb<false>(SuperVector<16> b)
{
    /* On Intel, if bit 0x80 is set, then result is zero, otherwise which the lane it is &0xf.
       In NEON or PPC, if >=16, then the result is zero, otherwise it is that lane.
       below is the version that is converted from Intel to PPC.  */
    uint8x16_t mask =(uint8x16_t)vec_cmpge(b.u.u8x16[0], (uint8x16_t)vec_splats((uint8_t)0x80));
    uint8x16_t res = vec_perm (u.u8x16[0], u.u8x16[0], b.u.u8x16[0]);
    return (m128) vec_sel(res, (uint8x16_t)vec_splat_s8(0), mask);
}

template<>
template<>
really_inline SuperVector<16> SuperVector<16>::pshufb<true>(SuperVector<16> b)
{
    /* On Intel, if bit 0x80 is set, then result is zero, otherwise which the lane it is &0xf.
       In NEON or PPC, if >=16, then the result is zero, otherwise it is that lane.
       btranslated is the version that is converted from Intel to PPC.  */
    SuperVector<16> btranslated = b & SuperVector<16>::dup_s8(0x8f);
    return pshufb<false>(btranslated);
}


template<>
really_inline SuperVector<16> SuperVector<16>::pshufb_maskz(SuperVector<16> b, uint8_t const len)
{
    SuperVector<16> mask = Ones_vshr(16 -len);
    return mask & pshufb(b);
}

#endif
