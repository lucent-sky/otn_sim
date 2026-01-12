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
    EXPECT_EQ(otu.odu_level(), static_cast<uint8_t>(OduLevel::ODU2));
}
