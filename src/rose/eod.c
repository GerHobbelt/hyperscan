/*
 * Copyright (c) 2015-2016, Intel Corporation
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

#include "catchup.h"
#include "match.h"
#include "program_runtime.h"
#include "rose.h"
#include "util/fatbit.h"

static really_inline
void initContext(const struct RoseEngine *t, u64a offset,
                 struct hs_scratch *scratch) {
    struct RoseContext *tctxt = &scratch->tctxt;
    /* TODO: diff groups for eod */
    tctxt->groups = loadGroups(t, scratch->core_info.state);
    tctxt->lit_offset_adjust = scratch->core_info.buf_offset
                             - scratch->core_info.hlen
                             + 1; // index after last byte
    tctxt->delayLastEndOffset = offset;
    tctxt->lastEndOffset = offset;
    tctxt->filledDelayedSlots = 0;
    tctxt->lastMatchOffset = 0;
    tctxt->minMatchOffset = offset;
    tctxt->minNonMpvMatchOffset = offset;
    tctxt->next_mpv_offset = offset;

    scratch->catchup_pq.qm_size = 0;
    scratch->al_log_sum = 0; /* clear the anchored logs */

    fatbit_clear(scratch->aqa);
}

static rose_inline
hwlmcb_rv_t roseEodRunMatcher(const struct RoseEngine *t, u64a offset,
                              struct hs_scratch *scratch,
                              const char is_streaming) {
    assert(t->ematcherOffset);

    size_t eod_len;
    const u8 *eod_data;
    if (!is_streaming) { /* Block */
        eod_data = scratch->core_info.buf;
        eod_len = scratch->core_info.len;
    } else { /* Streaming */
        eod_len = scratch->core_info.hlen;
        eod_data = scratch->core_info.hbuf;
    }

    assert(eod_data);
    assert(eod_len);

    // If we don't have enough bytes to produce a match from an EOD table scan,
    // there's no point scanning.
    if (eod_len < t->eodmatcherMinWidth) {
        DEBUG_PRINTF("len=%zu < eodmatcherMinWidth=%u\n", eod_len,
                     t->eodmatcherMinWidth);
        return HWLM_CONTINUE_MATCHING;
    }

    // Ensure that we only need scan the last N bytes, where N is the length of
    // the eod-anchored matcher region.
    size_t adj = eod_len - MIN(eod_len, t->ematcherRegionSize);

    DEBUG_PRINTF("eod offset=%llu, eod length=%zu\n", offset, eod_len);

    struct RoseContext *tctxt = &scratch->tctxt;
    const struct HWLM *etable = getELiteralMatcher(t);

    hwlmExec(etable, eod_data, eod_len, adj, roseCallback, scratch,
             tctxt->groups);

    // We may need to fire delayed matches
    return cleanUpDelayed(t, scratch, 0, offset);
}

static rose_inline
int roseEodRunIterator(const struct RoseEngine *t, u64a offset,
                       struct hs_scratch *scratch) {
    if (!t->eodIterProgramOffset) {
        return MO_CONTINUE_MATCHING;
    }

    DEBUG_PRINTF("running eod program at offset %u\n", t->eodIterProgramOffset);

    const u64a som = 0;
    const size_t match_len = 0;
    const char in_anchored = 0;
    const char in_catchup = 0;
    const char from_mpv = 0;
    const char skip_mpv_catchup = 1;
    if (roseRunProgram(t, scratch, t->eodIterProgramOffset, som, offset,
                       match_len, in_anchored, in_catchup,
                       from_mpv, skip_mpv_catchup) == HWLM_TERMINATE_MATCHING) {
        return MO_HALT_MATCHING;
    }

    return MO_CONTINUE_MATCHING;
}

static rose_inline
void cleanupAfterEodMatcher(const struct RoseEngine *t, u64a offset,
                            struct hs_scratch *scratch) {
    // Flush history to make sure it's consistent.
    roseFlushLastByteHistory(t, scratch, offset);
}

static rose_inline
int roseRunEodProgram(const struct RoseEngine *t, u64a offset,
                      struct hs_scratch *scratch) {
    if (!t->eodProgramOffset) {
        return MO_CONTINUE_MATCHING;
    }

    DEBUG_PRINTF("running eod program at %u\n", t->eodProgramOffset);

    // There should be no pending delayed literals.
    assert(!scratch->tctxt.filledDelayedSlots);

    const u64a som = 0;
    const size_t match_len = 0;
    const char in_anchored = 0;
    const char in_catchup = 0;
    const char from_mpv = 0;
    const char skip_mpv_catchup = 1;
    if (roseRunProgram(t, scratch, t->eodProgramOffset, som, offset, match_len,
                       in_anchored, in_catchup, from_mpv,
                       skip_mpv_catchup) == HWLM_TERMINATE_MATCHING) {
        return MO_HALT_MATCHING;
    }

    return MO_CONTINUE_MATCHING;
}

static really_inline
void roseEodExec_i(const struct RoseEngine *t, u64a offset,
                   struct hs_scratch *scratch, const char is_streaming) {
    assert(t);
    assert(scratch->core_info.buf || scratch->core_info.hbuf);
    assert(!scratch->core_info.buf || !scratch->core_info.hbuf);
    assert(!can_stop_matching(scratch));

    // Run the unconditional EOD program.
    if (roseRunEodProgram(t, offset, scratch) == MO_HALT_MATCHING) {
        return;
    }

    // Run the EOD anchored matcher if there is one.
    if (t->ematcherOffset) {
        assert(t->ematcherRegionSize);
        // Unset the reports we just fired so we don't fire them again below.
        char *state = scratch->core_info.state;
        mmbit_clear(getRoleState(state), t->rolesWithStateCount);
        mmbit_clear(getActiveLeafArray(t, state), t->activeArrayCount);

        if (roseEodRunMatcher(t, offset, scratch, is_streaming) ==
            HWLM_TERMINATE_MATCHING) {
            return;
        }

        cleanupAfterEodMatcher(t, offset, scratch);

        // Fire any new EOD reports.
        if (roseEodRunIterator(t, offset, scratch) == MO_HALT_MATCHING) {
            return;
        }
    }
}

void roseEodExec(const struct RoseEngine *t, u64a offset,
                 struct hs_scratch *scratch) {
    assert(scratch);
    assert(t->requiresEodCheck);
    DEBUG_PRINTF("ci buf %p/%zu his %p/%zu\n", scratch->core_info.buf,
                 scratch->core_info.len, scratch->core_info.hbuf,
                 scratch->core_info.hlen);

    // We should not have been called if we've already been told to terminate
    // matching.
    assert(!told_to_stop_matching(scratch));

    if (t->maxBiAnchoredWidth != ROSE_BOUND_INF
        && offset > t->maxBiAnchoredWidth) {
        DEBUG_PRINTF("bailing, we are beyond max width\n");
        /* also some of the history/state may be stale */
        return;
    }

    initContext(t, offset, scratch);
    roseEodExec_i(t, offset, scratch, 1);
}

static rose_inline
void prepForEod(const struct RoseEngine *t, struct hs_scratch *scratch,
                size_t length) {
    roseFlushLastByteHistory(t, scratch, length);
    scratch->tctxt.lastEndOffset = length;
}

void roseBlockEodExec(const struct RoseEngine *t, u64a offset,
                      struct hs_scratch *scratch) {
    assert(t->requiresEodCheck);
    assert(t->maxBiAnchoredWidth == ROSE_BOUND_INF
           || offset <= t->maxBiAnchoredWidth);

    assert(!can_stop_matching(scratch));

    // Ensure that history is correct before we look for EOD matches
    prepForEod(t, scratch, scratch->core_info.len);
    roseEodExec_i(t, offset, scratch, 0);
}
