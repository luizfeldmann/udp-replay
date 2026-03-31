// Local project
#include "version.h"
#include "CCommandLineArguments.h"

// Dependencies
#include <pcapplusplus/PcapLiveDeviceList.h>
#include <pcapplusplus/PcapFileDevice.h>
#include <pcapplusplus/UdpLayer.h>
#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/IPv6Layer.h>
#include <pcapplusplus/EthLayer.h>

// STD
#include <iostream>

//! Prints the network interfaces
void print_interfaces()
{
    static const char *szIndent = "    ";
    auto const &vDevices = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDevicesList();
    for (auto const &device : vDevices)
    {
        // Print name
        std::cout << device->getName() << std::endl;

        // Print the description
        std::string strDesc = device->getDesc();
        if (!strDesc.empty())
            std::cout << szIndent << strDesc << std::endl;

        // Print if loopback
        if (device->getLoopback())
            std::cout << szIndent << "Loopback" << std::endl;

        // Print MAC
        const auto macaddr = device->getMacAddress();
        if (macaddr.Zero != macaddr)
            std::cout << szIndent << "MAC: " << macaddr.toString() << std::endl;

        // Print gateway
        const auto gateway = device->getDefaultGateway();
        if (gateway.toInt() != 0)
            std::cout << szIndent << "Gateway: " << gateway << std::endl;

        // Print the addresses
        const auto addresses = device->getIPAddresses();
        if (!addresses.empty())
            std::cout << szIndent << "Address:" << std::endl;
        for (auto const &addr : addresses)
            std::cout << szIndent << szIndent << addr.toString() << std::endl;
    }
}

//! App entry point
int main(int argc, const char **argv)
{
    // Print header
    std::cout << version_name() << " (" << version_number() << ")" << std::endl;

    // Parse the CLI
    CCommandLineArguments cliArgs;
    std::error_code ec = cliArgs.parse(argc, argv);

    // Handle error in parsing the arguments
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return EXIT_FAILURE;
    }

    // Handle the operation requested in the CLI
    switch (cliArgs.get_operation())
    {
    default:
    case ECommandLineOperation::INVALID:
        return EXIT_FAILURE;

    case ECommandLineOperation::HELP:
        std::cout << cliArgs.get_help() << std::endl;
        break;

    case ECommandLineOperation::VERSION:
        std::cout << "Built: " << build_timestamp() << std::endl;
        std::cout << "Homepage: " << project_homepage_url() << std::endl;
        std::cout << "License: " << project_license_name() << std::endl;
        break;

    case ECommandLineOperation::LIST_INTERFACES:
        print_interfaces();
        break;

    case ECommandLineOperation::JOB:
        // TODO
        break;
    }

    return EXIT_SUCCESS;
}
