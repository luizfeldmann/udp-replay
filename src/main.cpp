// Local project
#include "version.h"
#include "CCommandLineArguments.h"
#include "CReplay.h"

// STD
#include <iostream>

//! App entry point
int main(int argc, const char **argv)
{
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
        std::cout << version_name() << " (v" << version_number() << ")" << std::endl;
        std::cout << cliArgs.get_help() << std::endl;
        break;

    case ECommandLineOperation::VERSION:
        std::cout << version_name() << std::endl;
        std::cout << "v" << version_number() << std::endl;
        std::cout << "Built: " << build_timestamp() << std::endl;
        std::cout << "Homepage: " << project_homepage_url() << std::endl;
        std::cout << "License: " << project_license_name() << std::endl;
        break;

    case ECommandLineOperation::LIST_INTERFACES:
        CReplay::print_interfaces();
        break;

    case ECommandLineOperation::JOB:
        CReplay replay(cliArgs.get_job_arguments());
        ec = replay.run_replay();
        if (ec)
        {
            std::cerr << ec.message() << std::endl;
            return EXIT_FAILURE;
        }
        break;
    }

    // Completed without any error :)
    return EXIT_SUCCESS;
}
