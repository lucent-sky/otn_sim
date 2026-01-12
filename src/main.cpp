#include <iostream>
#include "otn/otu.hpp"

int main() {
    using namespace otn;

    Payload payload(1500);
    Opu opu(payload);
    Odu odu(opu, 2);
    Otu otu(odu, true);

    std::cout << "OTN Simulator\n";
    std::cout << "Payload size: " << otu.payload_size() << " bytes\n";
    std::cout << "ODU level: ODU" << static_cast<int>(otu.odu_level()) << "\n";
    std::cout << "FEC enabled: " << (otu.fec_enabled() ? "yes" : "no") << "\n";

    return 0;
}
