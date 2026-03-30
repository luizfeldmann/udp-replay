// Local project
#include "version.h"

// Dependencies
#include <cxxopts.hpp>
#include <pcapplusplus/PcapLiveDeviceList.h>
#include <pcapplusplus/PcapFileDevice.h>
#include <pcapplusplus/UdpLayer.h>
#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/IPv6Layer.h>
#include <pcapplusplus/EthLayer.h>
// STD
#include <iostream>

//! App entry point
int main(int argc, char **argv)
{
    // Print header
    std::cout << version_name() << " (" << version_number() << ")" << std::endl;
}
