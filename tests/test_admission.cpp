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

TEST(AdmitCandidates, CostAwarePrefersLowerFragmentation) {
    Odu a(OduLevel::ODU1, 100);
    Odu b(OduLevel::ODU1, 100);

    // Existing grooming creates a hole in the middle
    std::vector<GroomedChild> existing = {
        GroomedChild(&a, a.slots(), 0),
        GroomedChild(&b, b.slots(), 2) // gap at slot 1
    };

    Odu incoming(OduLevel::ODU1, 100);

    Candidate bad_choice {
        .child  = &incoming,
        .offset = 4,   // creates new gap
        .cost   = 0.0
    };

    Candidate good_choice {
        .child  = &incoming,
        .offset = 1,   // fills the gap
        .cost   = 0.0
    };

    auto result = admit_candidates(
        OduLevel::ODU2,
        existing,
        { bad_choice, good_choice }
    );

    ASSERT_EQ(result.size(), 3u);

    // Incoming should fill the gap, not extend fragmentation
    EXPECT_EQ(result[2].slot_offset, 1u);
}

TEST(AdmitCandidates, CostAwareAdmissionIgnoresCandidateOrder) {
    Odu a(OduLevel::ODU1, 100);

    std::vector<GroomedChild> existing = {
        GroomedChild(&a, a.slots(), 0)
    };

    Odu b(OduLevel::ODU1, 100);

    Candidate worse {
        .child  = &b,
        .offset = 2,
        .cost   = 0.0
    };

    Candidate better {
        .child  = &b,
        .offset = 1,
        .cost   = 0.0
    };

    // Worse candidate comes first intentionally
    auto result = admit_candidates(
        OduLevel::ODU2,
        existing,
        { worse, better }
    );

    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[1].slot_offset, 1u);
}

TEST(AdmitCandidates, CostAwareBehavesLikeGreedyWhenNoFragmentationDifference) {
    Odu a(OduLevel::ODU1, 100);
    Odu b(OduLevel::ODU1, 100);

    std::vector<Candidate> candidates = {
        { &a, 0, 0.0 },
        { &b, 1, 0.0 }
    };

    auto result = admit_candidates(
        OduLevel::ODU2,
        {},
        candidates
    );

    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].slot_offset, 0u);
    EXPECT_EQ(result[1].slot_offset, 1u);
}

TEST(AdmitCandidates, CostTieBreakIsStable) {
    Odu a(OduLevel::ODU1, 100);
    Odu b(OduLevel::ODU1, 100);

    Candidate c1 { &a, 0, 0.0 };
    Candidate c2 { &b, 0, 0.0 };

    auto result = admit_candidates(
        OduLevel::ODU2,
        {},
        { c1, c2 }
    );

    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].child, &a);
    EXPECT_EQ(result[1].child, &b);
}
