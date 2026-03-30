// Local
#include "parsecli.h"
#include "version.h"

// Dependencies
#include <cxxopts.hpp>

// STD
#include <ranges>
#include <iostream>
#include <charconv>
#include <unordered_map>

//! Implements the accessors to the command line arguments
class CCommandLineArguments
    : public ICommandLineArguments,
      private IJobArguments
{
private:
    //! The selected operation
    ECommandLineOperation m_op = ECommandLineOperation::INVALID;

    //! Repeat mode
    ERepeatMode m_eRepeat = ERepeatMode::NO_REPEAT;

    //! Number of times to repeat
    unsigned int m_uRepeatTimes = 0;

    //! Speed mode
    ESpeedMode m_eSpeed = ESpeedMode::NO_SPEED;

    //! Speed multiplifer factor
    double m_dSpeedMultiplier = 1.0;

    //! Limit mode
    ELimitMode m_eLimit = ELimitMode::NO_LIMIT;

    //! Time limitation
    unsigned int m_uLimitTime = 0;

    //! Number of packets limitation
    unsigned int m_uLimitPackets = 0;

    //! Indicates verbose mode
    bool m_bVerbose = false;

    //! The help text
    std::string m_strHelp;

    //! The file name of the capture
    std::string m_strFilePath;

    //! The name of the network interface to use
    std::string m_strInterface;

    //! Maps original to rewritten port numbers
    std::unordered_map<unsigned short, unsigned short> m_mapPorts;

public:
    //! Constructor
    CCommandLineArguments() = default;

    //! Parses the command line arguments
    void parse(int argc, const char **argv)
    {
        bool bShowHelp = false;
        bool bShowVersion = false;
        bool bShowInterfaces = false;
        std::string strPortRemap;
        std::string strDestRemap;

        static const char *szFilenameFlag = "file";

        // Define args
        cxxopts::Options options(version_name(), version_description());
        cxxopts::OptionAdder add = options.add_options();
        add(szFilenameFlag, "The input capture file path", cxxopts::value(m_strFilePath));
        add("r,portmap", "Rewrite port numbers", cxxopts::value(strPortRemap));
        add("D,dstipmap", "Rewrite destination IP addresses", cxxopts::value(strPortRemap));
        add("i,intf1", "Selects the interface to use", cxxopts::value(m_strInterface));
        add("l,loop", "Loop through the capture file X times", cxxopts::value(m_uRepeatTimes));
        add("L,limit", "Limit the number of packets to send", cxxopts::value(m_uLimitPackets));
        add("duration", "Limit the number of seconds to send", cxxopts::value(m_uLimitTime));
        add("x,multiplier", "Modify replay speed to a given multiple", cxxopts::value(m_dSpeedMultiplier));
        add("I,interfaces", "Lists the available network interfaces", cxxopts::value(bShowInterfaces));
        add("v,verbose", "Print packet information and progress", cxxopts::value(m_bVerbose));
        add("V,version", "Print version information", cxxopts::value(bShowVersion));
        add("h,help", "Display usage information and exit", cxxopts::value(bShowHelp));

        options.parse_positional({szFilenameFlag});

        // Invoke CLI parser
        cxxopts::ParseResult parsed(options.parse(argc, argv));

        // Check combination of mutually exclusive options
        if ((bShowHelp || bShowVersion || bShowInterfaces) && parsed.arguments().size() > 1)
            throw std::invalid_argument("combining mutually exclusive options");

        // Check the modes used to print and exit
        if (bShowHelp)
        {
            m_op = ECommandLineOperation::HELP;
            m_strHelp = options.help();
            return;
        }

        if (bShowVersion)
        {
            m_op = ECommandLineOperation::VERSION;
            return;
        }

        if (bShowInterfaces)
        {
            m_op = ECommandLineOperation::LIST_INTERFACES;
            return;
        }

        // Validate job arguments
        if (m_strFilePath.empty())
            throw std::invalid_argument("requires input file path");

        // Validate the speed mode
        if (parsed.contains("multiplier"))
            m_eSpeed = ESpeedMode::SPEED_MULTIPLIER;

        // Validate the repeat mode
        if (parsed.contains("loop"))
        {
            if (m_uRepeatTimes == 0)
                m_eRepeat = ERepeatMode::LOOP;
            else
                m_eRepeat = ERepeatMode::REPEAT_TIMES;
        }

        // Validate the limitation mode
        const bool bLimitPackets = parsed.contains("limit");
        const bool bLimitDuration = parsed.contains("duration");

        if (bLimitDuration && bLimitPackets)
            throw std::invalid_argument("cannot specify both duration limit and number of packets limit");
        else if (bLimitDuration)
            m_eLimit = ELimitMode::LIMIT_MAX_TIME;
        else if (bLimitPackets)
            m_eLimit = ELimitMode::LIMIT_MAX_PACKETS;

        // Parse the port remap
        auto splitPortMappings = std::views::split(strPortRemap, ',');
        for (auto &&portMapping : splitPortMappings)
        {
            // Find the colon delimiter
            std::string_view svPortEntry(portMapping.begin(), portMapping.end());
            const size_t nColon = svPortEntry.find(':');
            if (std::string_view::npos == nColon)
                throw std::invalid_argument("each port pair must be delimited by a colon ('from:to')");

            // Right side
            unsigned short usToPort = 0;
            const std::from_chars_result rightMatch = std::from_chars(svPortEntry.begin() + nColon + 1, svPortEntry.end(), usToPort);
            if (std::errc{} != rightMatch.ec || rightMatch.ptr != svPortEntry.end())
                throw std::invalid_argument("invalid rewritten port");

            // Left side can be a single port or a range
            unsigned usFromPort1 = 0;
            const std::from_chars_result leftMatch = std::from_chars(svPortEntry.begin(), svPortEntry.begin() + nColon, usFromPort1);
            if (std::errc{} != leftMatch.ec)
                throw std::invalid_argument("invalid matched port");

            if (*leftMatch.ptr == '-')
            {
                // Left side is a range
                unsigned usFromPort2 = 0;
                const std::from_chars_result middleMatch = std::from_chars(leftMatch.ptr + 1, svPortEntry.begin() + nColon, usFromPort2);
                if (std::errc{} != middleMatch.ec || middleMatch.ptr != svPortEntry.begin() + nColon)
                    throw std::invalid_argument("invalid rewritten port");

                if (usFromPort2 <= usFromPort1)
                    throw std::invalid_argument("end port of range must be larger than start port");

                // Store the full range in the map
                for (unsigned p = usFromPort1; p <= usFromPort2; ++p)
                    m_mapPorts[p] = usToPort;
            }
            else if (leftMatch.ptr == svPortEntry.begin() + nColon)
            {
                // Left side is a single port
                m_mapPorts[usFromPort1] = usToPort;
            }
        }

        // The job is valid
        m_op = ECommandLineOperation::JOB;
    }

    ECommandLineOperation get_operation() const override
    {
        return m_op;
    }

    std::string_view get_help() const override
    {
        return m_strHelp;
    }

    IJobArguments const &get_job_arguments() const override
    {
        if (m_op != ECommandLineOperation::JOB)
            throw std::runtime_error("job not specified");
        return *this;
    }

    bool is_verbose() const override
    {
        return m_bVerbose;
    }

    std::string_view get_filename() const override
    {
        return m_strFilePath;
    }

    std::string_view get_interface_name() const
    {
        return m_strInterface;
    }

    ERepeatMode get_repeat_mode() const override
    {
        return m_eRepeat;
    }

    unsigned int get_repeat_count() const override
    {
        if (m_eRepeat != ERepeatMode::REPEAT_TIMES)
            throw std::runtime_error("repeat count not defined");
        return m_uRepeatTimes;
    }

    ELimitMode get_limit_mode() const override
    {
        return m_eLimit;
    }

    unsigned int get_limit_packets() const override
    {
        if (m_eLimit != ELimitMode::LIMIT_MAX_PACKETS)
            throw std::runtime_error("packets limit not defined");
        return m_uLimitPackets;
    }

    unsigned int get_limit_duration() const override
    {
        if (m_eLimit != ELimitMode::LIMIT_MAX_TIME)
            throw std::runtime_error("duration limit not defined");
        return m_uLimitTime;
    }

    ESpeedMode get_speed_mode() const override
    {
        return m_eSpeed;
    }

    double get_speed_multiplier() const override
    {
        if (m_eSpeed != ESpeedMode::SPEED_MULTIPLIER)
            throw std::runtime_error("speed multiplier not defined");
        return m_dSpeedMultiplier;
    }

    unsigned short get_port_remap(unsigned short usOriginalPort) const override
    {
        const auto it = m_mapPorts.find(usOriginalPort);
        if (it != m_mapPorts.cend())
            return it->second;

        // Return original port if not remapping is found
        return usOriginalPort;
    }

    std::string_view get_ipv4_destination_remap(std::string_view dest_ipv4) const override
    {
        return dest_ipv4; // TODO
    }

    std::string_view get_ipv6_destination_remap(std::string_view dest_ipv6) const override
    {
        return dest_ipv6; // TODO
    }
};

std::unique_ptr<ICommandLineArguments> parse_cli(int argc, const char **argv)
{
    // Arguments holder with default values
    auto pArgs = std::make_unique<CCommandLineArguments>();

    // Safely parse the input
    try
    {
        pArgs->parse(argc, argv);
    }
    catch (std::exception const &ex)
    {
        std::cerr << "arguments error: " << ex.what() << std::endl;
    }

    return pArgs;
}
