#include <gtest/gtest.h>

#include "otn/payload.hpp"
#include "otn/opu.hpp"
#include "otn/odu.hpp"
#include "otn/otu.hpp"
#include "otn/fragmentation.hpp"
#include "otn/grooming_planner.hpp"

using namespace otn;

// ---------------- Payload ----------------

TEST(PayloadTest, SizeIsCorrect) {
    Payload p(1000);
    EXPECT_EQ(p.size(), 1000);
}

// ---------------- OPU ----------------

TEST(OpuTest, PayloadPassThrough) {
    Payload p(500);
    Opu opu(p);
    EXPECT_EQ(opu.payload_size(), 500);
}

// ---------------- ODU ----------------

TEST(OduTest, LeafOduHasCorrectLevelAndPayload) {
    Odu odu(OduLevel::ODU2, 200);

    EXPECT_EQ(odu.level(), OduLevel::ODU2);
    EXPECT_EQ(odu.payload_size(), 200);
    EXPECT_FALSE(odu.is_aggregated());
}

TEST(OduTest, AggregatedOduSumsChildren) {
    Odu child1(OduLevel::ODU1, 100);
    Odu child2(OduLevel::ODU1, 150);

    std::vector<GroomedChild> groomed = {
        GroomedChild(&child1, child1.slots(), 0),
        GroomedChild(&child2, child2.slots(), 1)
    };
    Odu parent(OduLevel::ODU2, groomed);

    EXPECT_EQ(parent.level(), OduLevel::ODU2);
    EXPECT_EQ(parent.payload_size(), 250);
    EXPECT_TRUE(parent.is_aggregated());
}

// ---------------- OTU ----------------

TEST(OtuTest, OtuWrapsOduCorrectly) {
    Odu odu(OduLevel::ODU2, 300);
    Otu otu(odu, true);

    EXPECT_TRUE(otu.fec_enabled());
    EXPECT_EQ(otu.payload_size(), 300);
    EXPECT_EQ(otu.odu_level(), OduLevel::ODU2);
}


TEST(OduTest, CanConstructFromOpu) {
    Payload p(400);
    Opu opu(p);
    Odu odu(OduLevel::ODU2, opu);
    EXPECT_EQ(odu.payload_size(), 400);
}

// ---------------- Nested Aggregation test ----------------
TEST(OduTest, NestedAggregationSumsCorrectly) {
    Odu leaf1(OduLevel::ODU1, 100);
    Odu leaf2(OduLevel::ODU1, 200);

    std::vector<GroomedChild> mid_children = {
        GroomedChild(&leaf1, leaf1.slots(), 0),
        GroomedChild(&leaf2, leaf2.slots(), 1)
    };
    Odu mid(OduLevel::ODU2, mid_children);

    std::vector<GroomedChild> top_children = {
        GroomedChild(&mid, mid.slots(), 0)
    };
    Odu top(OduLevel::ODU3, top_children);

    EXPECT_EQ(top.payload_size(), 300);
    EXPECT_TRUE(top.is_aggregated());
}

// ---------------- Aggregated ODU test ----------------
TEST(OduTest, AggregatedOduIsNotLeaf) {
    Odu child(OduLevel::ODU1, 100);

    std::vector<GroomedChild> groomed = {
        GroomedChild(&child, child.slots(), 0)
    };
    Odu parent(OduLevel::ODU2, groomed);

    EXPECT_TRUE(parent.is_aggregated());
    EXPECT_NE(parent.payload_size(), 0u);
}


// ---------------- Anti-regression test ----------------
TEST(OtuTest, FecDoesNotChangePayloadSize) {
    Odu odu(OduLevel::ODU2, 500);

    Otu otu_no_fec(odu, false);
    Otu otu_fec(odu, true);

    EXPECT_EQ(otu_no_fec.payload_size(), otu_fec.payload_size());
}

// ---------------- 0 payload edge-case test ----------------
TEST(PayloadTest, ZeroSizePayloadIsValid) {
    Payload p(0);
    EXPECT_EQ(p.size(), 0u);
}

// ---------------- ODU level integrity test ----------------
TEST(OtuTest, OduLevelIsPreserved) {
    Odu odu(OduLevel::ODU3, 123);
    Otu otu(odu, false);

    EXPECT_EQ(otu.odu_level(), OduLevel::ODU3);
}

// ---------------- FEC flag integrity test ----------------
TEST(OtuTest, FecFlagDoesNotAffectPayloadSize) {
    Odu odu(OduLevel::ODU2, 100);
    Otu otu_no_fec(odu, false);
    Otu otu_fec(odu, true);

    EXPECT_EQ(otu_no_fec.payload_size(), 100);
    EXPECT_EQ(otu_fec.payload_size(), 100);
}

// ---------------- Deprecated Nominal capacities tests ----------------
/* DEPRECATED: nominal capacities removed
TEST(OduCapacityTest, NominalCapacitiesExist) {
    EXPECT_GT(nominal_capacity(OduLevel::ODU1), 0);
    EXPECT_GT(nominal_capacity(OduLevel::ODU4), nominal_capacity(OduLevel::ODU2));
}

TEST(OduCapacityTest, LeafCannotExceedNominalCapacity) {
    EXPECT_THROW(
        Odu(OduLevel::ODU1, nominal_capacity(OduLevel::ODU1) + 1),
        std::runtime_error
    );
}

TEST(OduCapacityTest, AggregatedCannotExceedNominalCapacity) {
    Odu c1(OduLevel::ODU1, 2000);
    Odu c2(OduLevel::ODU1, 2000);

    EXPECT_THROW(
        Odu(OduLevel::ODU1, {c1, c2}),
        std::runtime_error
    );
} */

// ---------------- Deprecated Mux tests ----------------
/* DEPRECATED - mux has been changed to include grooming
TEST(MuxTest, ValidMuxSucceeds) { ... }
TEST(MuxTest, InvalidHierarchyFails) { ... }
TEST(MuxTest, CapacityOverflowFails) { ... }
TEST(MuxTest, CapacityBoundarySucceeds) { ... }
TEST(MuxTest, NonAdjacentHierarchyFails) { ... }
TEST(MuxTest, MixedChildLevelsFail) { ... }
*/

// ---------------- Updated Explicit Grooming Tests ----------------

TEST(GroomingTest, ValidExplicitGroomingSucceeds) {
    Odu c1(OduLevel::ODU1, 100);
    Odu c2(OduLevel::ODU1, 200);

    std::vector<GroomedChild> groomed = {
        GroomedChild(&c1, c1.slots(), 0),
        GroomedChild(&c2, c2.slots(), 1)
    };
    Odu parent(OduLevel::ODU2, groomed);

    EXPECT_EQ(parent.slots(), 2u);
    EXPECT_EQ(parent.payload_size(), 300u);
    EXPECT_TRUE(parent.is_aggregated());
}

TEST(GroomingTest, OverlappingSlotsFail) {
    Odu c1(OduLevel::ODU1, 100);
    Odu c2(OduLevel::ODU1, 200);

    std::vector<GroomedChild> groomed = {
        GroomedChild(&c1, c1.slots(), 0),
        GroomedChild(&c2, c2.slots(), 0) // overlap
    };

    EXPECT_THROW(
        Odu(OduLevel::ODU2, groomed),
        std::runtime_error
    );
}

TEST(GroomingTest, SlotOverflowFails) {
    Odu c1(OduLevel::ODU1, 100);
    Odu c2(OduLevel::ODU1, 100);
    Odu c3(OduLevel::ODU1, 100);
    Odu c4(OduLevel::ODU1, 100);
    Odu c5(OduLevel::ODU1, 100); // 5 slots

    std::vector<GroomedChild> groomed = {
        GroomedChild(&c1, c1.slots(), 0),
        GroomedChild(&c2, c2.slots(), 1),
        GroomedChild(&c3, c3.slots(), 2),
        GroomedChild(&c4, c4.slots(), 3),
        GroomedChild(&c5, c5.slots(), 4) // out of bounds
    };

    EXPECT_THROW(
        Odu(OduLevel::ODU2, groomed),
        std::runtime_error
    );
}

TEST(GroomingTest, NonAdjacentHierarchyFails) {
    Odu c1(OduLevel::ODU1, 100);
    std::vector<GroomedChild> groomed = {
        GroomedChild(&c1, c1.slots(), 0)
    };

    EXPECT_THROW(
        Odu(OduLevel::ODU3, groomed),
        std::runtime_error
    );
}

TEST(FragmentationTest, MetricsAreComputedCorrectly) {
    Odu c1(OduLevel::ODU1, 100);
    Odu c2(OduLevel::ODU1, 100);
    Odu c3(OduLevel::ODU1, 100);

    std::vector<GroomedChild> grooming = {
        GroomedChild(&c1, c1.slots(), 0),
        GroomedChild(&c2, c2.slots(), 2), // gap at slot 1
        GroomedChild(&c3, c3.slots(), 4)  // gap at slot 3
    };

    auto m = analyze_fragmentation(grooming);

    EXPECT_EQ(m.gap_count, 2u);
    EXPECT_EQ(m.total_gap_slots, 2u);
    EXPECT_EQ(m.max_gap, 1u);
    EXPECT_EQ(m.span_slots, 5u);
    EXPECT_LT(m.utilization, 1.0);
}


TEST(GroomingPlannerTest, SizeAwareRepackProducesValidGrooming) {
    Odu small(OduLevel::ODU1, 100);
    Odu large(OduLevel::ODU1, 300);

    std::vector<GroomedChild> grooming = {
        GroomedChild(&small, small.slots(), 2),
        GroomedChild(&large, large.slots(), 0)
    };

    auto repacked = repack_grooming_size_aware(
        OduLevel::ODU2,
        grooming
    );

    EXPECT_EQ(repacked.size(), 2u);
    EXPECT_EQ(repacked[0].child, &large);
    EXPECT_EQ(repacked[0].slot_offset, 0u);
}

TEST(GroomingPlannerTest, SizeAwareRepackReducesFragmentation) {
    Odu a(OduLevel::ODU1, 100);
    Odu b(OduLevel::ODU1, 300);
    Odu c(OduLevel::ODU1, 100);

    std::vector<GroomedChild> fragmented = {
        GroomedChild(&a, a.slots(), 0),
        GroomedChild(&b, b.slots(), 2), // fragmentation
        GroomedChild(&c, c.slots(), 6)
    };

    auto stable = repack_grooming(
        OduLevel::ODU2,
        fragmented
    );

    auto size_aware = repack_grooming_size_aware(
        OduLevel::ODU2,
        fragmented
    );

    auto m_stable = analyze_fragmentation(stable);
    auto m_size   = analyze_fragmentation(size_aware);

    EXPECT_LE(m_size.max_gap, m_stable.max_gap);
    EXPECT_GE(m_size.utilization, m_stable.utilization);
}


TEST(GroomingPlannerTest, DeterministicRepackProducesValidGrooming) {
    Odu small(OduLevel::ODU1, 100);
    Odu medium(OduLevel::ODU1, 200);
    Odu large(OduLevel::ODU1, 300);

    std::vector<GroomedChild> original = {
        GroomedChild(&small, small.slots(), 0),
        GroomedChild(&medium, medium.slots(), 2),
        GroomedChild(&large, large.slots(), 1)
    };

    auto repacked = otn::repack_grooming_deterministic(OduLevel::ODU2, original);

    // Verify no overlaps
    std::vector<bool> slot_map(tributary_slots(OduLevel::ODU2), false);
    for (const auto& g : repacked) {
        for (size_t i = 0; i < g.slot_width; ++i) {
            ASSERT_FALSE(slot_map[g.slot_offset + i]);
            slot_map[g.slot_offset + i] = true;
        }
    }

    // Verify utilization improved or unchanged
    auto before = otn::analyze_fragmentation(original);
    auto after = otn::analyze_fragmentation(repacked);

    EXPECT_GE(after.utilization, before.utilization);
}
