#include <gtest/gtest.h>

#include "otn/payload.hpp"
#include "otn/opu.hpp"
#include "otn/odu.hpp"
#include "otn/otu.hpp"

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
        {&child1, child1.slots(), 0},
        {&child2, child2.slots(), 1}
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
        {&leaf1, leaf1.slots(), 0},
        {&leaf2, leaf2.slots(), 1}
    };
    Odu mid(OduLevel::ODU2, mid_children);

    std::vector<GroomedChild> top_children = {
        {&mid, mid.slots(), 0}
    };
    Odu top(OduLevel::ODU3, top_children);

    EXPECT_EQ(top.payload_size(), 300);
    EXPECT_TRUE(top.is_aggregated());
}

// ---------------- Aggregated ODU test ----------------
TEST(OduTest, AggregatedOduIsNotLeaf) {
    Odu child(OduLevel::ODU1, 100);

    std::vector<GroomedChild> groomed = {
        {&child, child.slots(), 0}
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

    GroomedChild g1{&c1, c1.slots(), 0};
    GroomedChild g2{&c2, c2.slots(), 1};

    std::vector<GroomedChild> groomed = {g1, g2};
    Odu parent(OduLevel::ODU2, groomed);

    EXPECT_EQ(parent.slots(), 2u);
    EXPECT_EQ(parent.payload_size(), 300u);
    EXPECT_TRUE(parent.is_aggregated());
}

TEST(GroomingTest, OverlappingSlotsFail) {
    Odu c1(OduLevel::ODU1, 100);
    Odu c2(OduLevel::ODU1, 200);

    GroomedChild g1{&c1, c1.slots(), 0};
    GroomedChild g2{&c2, c2.slots(), 0}; // overlap

    std::vector<GroomedChild> groomed = {g1, g2};

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

    GroomedChild g1{&c1, c1.slots(), 0};
    GroomedChild g2{&c2, c2.slots(), 1};
    GroomedChild g3{&c3, c3.slots(), 2};
    GroomedChild g4{&c4, c4.slots(), 3};
    GroomedChild g5{&c5, c5.slots(), 4}; // out of bounds

    std::vector<GroomedChild> groomed = {g1, g2, g3, g4, g5};

    EXPECT_THROW(
        Odu(OduLevel::ODU2, groomed),
        std::runtime_error
    );
}

TEST(GroomingTest, NonAdjacentHierarchyFails) {
    Odu c1(OduLevel::ODU1, 100);
    GroomedChild g{&c1, c1.slots(), 0};

    std::vector<GroomedChild> groomed = {g};

    EXPECT_THROW(
        Odu(OduLevel::ODU3, groomed),
        std::runtime_error
    );
}

// ---------------- Grooming Planner Tests ----------------
/* plan_grooming not implemented yet; tests left for future implementation
TEST(GroomingPlannerTest, PlannerProducesValidGrooming) { ... }
TEST(GroomingPlannerTest, PlannerThrowsOnOverflow) { ... }
*/
