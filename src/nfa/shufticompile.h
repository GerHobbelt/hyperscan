/*
 * Copyright (c) 2015, Intel Corporation
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

/** \file
 * \brief Shufti acceleration: compile code.
 */

#ifndef SHUFTI_COMPILE_H
#define SHUFTI_COMPILE_H

#include "ue2common.h"
#include "util/charreach.h"
#include "util/ue2_containers.h"

#include <utility>

namespace ue2 {

/** \brief Single-byte variant.
 *
 * Returns -1 if unable to construct masks, otherwise returns number of bits
 * used in the mask.
 *
 * Note: always able to construct masks for 8 or fewer characters.
 */
int shuftiBuildMasks(const CharReach &chars, m128 *lo, m128 *hi);

void shuftiBuildDoubleMasks(const CharReach &onechar,
                            const flat_set<std::pair<u8, u8>> &twochar,
                            m128 *lo1, m128 *hi1, m128 *lo2, m128 *hi2);

void mergeShuftiMask(m128 *lo, const m128 lo_in, u32 lo_bits);

#ifdef DUMP_SUPPORT

/**
 * \brief Dump code: returns a CharReach with the reach that would match this
 * shufti.
 */
CharReach shufti2cr(const m128 lo, const m128 hi);

#endif // DUMP_SUPPORT

} // namespace ue2

#endif // SHUFTI_COMPILE_H