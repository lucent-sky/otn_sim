#include <iostream>
#include "otn/payload.hpp"
#include "otn/opu.hpp"
#include "otn/odu.hpp"
#include "otn/otu.hpp"

int main() {
    using namespace otn;

    Payload payload(1500);
    Opu opu(payload);
    Odu odu(OduLevel::ODU2, opu);
    Otu otu(odu, true);

    std::cout << "OTN Simulator\n";
    std::cout << "Payload size: " << otu.payload_size() << " bytes\n";
    std::cout << "ODU level: ODU" << static_cast<int>(otu.odu_level()) << "\n";
    std::cout << "FEC enabled: " << (otu.fec_enabled() ? "yes" : "no") << "\n";

    return 0;
}
