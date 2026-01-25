#include <gtest/gtest.h>

#include "otn/grooming_planner.hpp"
#include "otn/odu.hpp"
#include "otn/otn_types.hpp"

using namespace otn;

/*
 * Helper: create a leaf ODU with payload sized exactly to its slot count.
 * This avoids hard-coding payload bytes.
 */
static Odu make_leaf(OduLevel level) {
    return Odu(level, {});
}

TEST(AdmitCandidates, SingleCandidateFitsEmptyParent) {
    Odu child = make_leaf(OduLevel::ODU1);

    Candidate c {
        .child = &child,
        .offset = 0,
        .cost = 0.0
    };

    auto result = admit_candidates(
        OduLevel::ODU4,
        {},                 // empty grooming
        {c}
    );

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].child, &child);
    EXPECT_EQ(result[0].slot_offset, 0u);
    EXPECT_EQ(result[0].slot_width, child.slots());
}

TEST(AdmitCandidates, MultipleCandidatesPackLeftToRight) {
    Odu c1 = make_leaf(OduLevel::ODU1);
    Odu c2 = make_leaf(OduLevel::ODU1);
    Odu c3 = make_leaf(OduLevel::ODU1);

    std::vector<Candidate> candidates = {
        { &c1, 0, 0.0 },
        { &c2, c1.slots(), 0.0 },
        { &c3, 2 * c1.slots(), 0.0 }
    };

    auto result = admit_candidates(
        OduLevel::ODU4,
        {},
        candidates
    );

    ASSERT_EQ(result.size(), 3u);

    EXPECT_EQ(result[0].child, &c1);
    EXPECT_EQ(result[0].slot_offset, 0u);

    EXPECT_EQ(result[1].child, &c2);
    EXPECT_EQ(result[1].slot_offset, c1.slots());

    EXPECT_EQ(result[2].child, &c3);
    EXPECT_EQ(result[2].slot_offset, 2 * c1.slots());
}

TEST(AdmitCandidates, SkipsCandidateThatDoesNotFit) {
    Odu fits = make_leaf(OduLevel::ODU2);
    Odu too_big = make_leaf(OduLevel::ODU4);

    std::vector<Candidate> candidates = {
        { &fits, 0, 0.0 },
        { &too_big, fits.slots(), 0.0 }
    };

    auto result = admit_candidates(
        OduLevel::ODU4,
        {},
        candidates
    );

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].child, &fits);
}

TEST(AdmitCandidates, RespectsExistingGrooming) {
    Odu existing = make_leaf(OduLevel::ODU1);
    Odu incoming = make_leaf(OduLevel::ODU1);

    GroomedChild g_existing(&existing, 0);

    Candidate c {
        .child = &incoming,
        .offset = existing.slots(),
        .cost = 0.0
    };

    auto result = admit_candidates(
        OduLevel::ODU4,
        { g_existing },
        { c }
    );

    ASSERT_EQ(result.size(), 2u);

    EXPECT_EQ(result[0].child, &existing);
    EXPECT_EQ(result[0].slot_offset, 0u);

    EXPECT_EQ(result[1].child, &incoming);
    EXPECT_EQ(result[1].slot_offset, existing.slots());
}
