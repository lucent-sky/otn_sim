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

// ---------------- ODU (Day 3 model) ----------------

TEST(OduTest, LeafOduHasCorrectLevelAndPayload) {
    Odu odu(OduLevel::ODU2, 200);

    EXPECT_EQ(odu.level(), OduLevel::ODU2);
    EXPECT_EQ(odu.payload_size(), 200);
    EXPECT_FALSE(odu.is_aggregated());
}

TEST(OduTest, AggregatedOduSumsChildren) {
    Odu child1(OduLevel::ODU1, 100);
    Odu child2(OduLevel::ODU1, 150);

    Odu parent(OduLevel::ODU2, {child1, child2});

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

    Odu mid(OduLevel::ODU2, {leaf1, leaf2});
    Odu top(OduLevel::ODU3, {mid});

    EXPECT_EQ(top.payload_size(), 300);
    EXPECT_TRUE(top.is_aggregated());
}

// ---------------- Aggregated ODU test ----------------
TEST(OduTest, AggregatedOduIsNotLeaf) {
    Odu child(OduLevel::ODU1, 100);
    Odu parent(OduLevel::ODU2, {child});

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