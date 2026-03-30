// Local project
#include "version.h"
#include "parsecli.h"

// Dependencies
#include <pcapplusplus/PcapLiveDeviceList.h>
#include <pcapplusplus/PcapFileDevice.h>
#include <pcapplusplus/UdpLayer.h>
#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/IPv6Layer.h>
#include <pcapplusplus/EthLayer.h>

// STD
#include <iostream>

//! App entry point
int main(int argc, const char **argv)
{
    // Print header
    std::cout << version_name() << " (" << version_number() << ")" << std::endl;

    // Parse the CLI
    const std::unique_ptr<ICommandLineArguments> pCliArgs = parse_cli(argc, argv);

    // Handle the operation requested in the CLI
    switch (pCliArgs->get_operation())
    {
    default:
    case ECommandLineOperation::INVALID:
        return EXIT_FAILURE;

    case ECommandLineOperation::HELP:
        std::cout << pCliArgs->get_help() << std::endl;
        break;

    case ECommandLineOperation::VERSION:
        break;

    case ECommandLineOperation::LIST_INTERFACES:
        // TODO
        break;

    case ECommandLineOperation::JOB:
        // TODO
        break;
    }

    return EXIT_SUCCESS;
}
