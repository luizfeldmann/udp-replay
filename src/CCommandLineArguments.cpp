// Local
#include "CCommandLineArguments.h"
#include "EAppError.h"
#include "version.h"

// Dependencies
#include <cxxopts.hpp>
#include <pcapplusplus/SystemUtils.h>

// STD
#include <ranges>
#include <iostream>
#include <charconv>
#include <unordered_map>

CCommandLineArguments::CCommandLineArguments()
{
}

std::error_code CCommandLineArguments::parse_port_remap(std::string_view svPortRemap)
{
    auto splitPortMappings = std::views::split(svPortRemap, ',');
    for (auto &&portPair : splitPortMappings)
    {
        // Find the colon delimiter
        const std::string_view svPortEntry(portPair.begin(), portPair.end());
        const size_t nColon = svPortEntry.find(':');
        if (std::string_view::npos == nColon)
            return EAppError::InvalidPortDelimiter;

        // Right side
        unsigned short usToPort = 0;
        const std::from_chars_result rightMatch = std::from_chars(svPortEntry.begin() + nColon + 1, svPortEntry.end(), usToPort);
        if (std::errc{} != rightMatch.ec || rightMatch.ptr != svPortEntry.end())
            return EAppError::InvalidRewritePort;

        // Left side can be a single port or a range
        unsigned usFromPort1 = 0;
        const std::from_chars_result leftMatch = std::from_chars(svPortEntry.begin(), svPortEntry.begin() + nColon, usFromPort1);
        if (std::errc{} != leftMatch.ec)
            return EAppError::InvalidMatchPort;

        if (*leftMatch.ptr == '-')
        {
            // Left side is a range
            unsigned usFromPort2 = 0;
            const std::from_chars_result middleMatch = std::from_chars(leftMatch.ptr + 1, svPortEntry.begin() + nColon, usFromPort2);
            if (std::errc{} != middleMatch.ec || middleMatch.ptr != svPortEntry.begin() + nColon)
                return EAppError::InvalidRewritePort;

            if (usFromPort2 <= usFromPort1)
                return EAppError::InvalidPortRangeOrder;

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

    // No error
    return std::error_code{};
}

std::error_code CCommandLineArguments::parse_dest_addr_remap(std::string_view svDestAddrRemap)
{
    std::error_code ec;

    // Split comma separated entries
    auto splitAddrMappings = std::views::split(svDestAddrRemap, ',');
    for (auto &&addrPair : splitAddrMappings)
    {
        const std::string_view svAddrEntry(addrPair.begin(), addrPair.end());

        // Handle IPv6 in format [from]:[to]
        if (svAddrEntry.starts_with('['))
            ec = parse_addr_remap_v6(svAddrEntry);
        else
            ec = parse_addr_remap_v4(svAddrEntry);

        // Stop on first error
        if (ec)
            break;
    }

    return ec;
}

std::error_code CCommandLineArguments::parse_addr_remap_v4(const std::string_view svAddrEntryV4)
{
    const size_t nDelim = svAddrEntryV4.find(':');
    if (std::string_view::npos == nDelim)
        return EAppError::InvalidAddressDelimiter;

    const std::string left(svAddrEntryV4.begin(), svAddrEntryV4.begin() + nDelim);
    const std::string right(svAddrEntryV4.begin() + nDelim + 1, svAddrEntryV4.end());

    pcpp::IPv4Network netFrom(left);
    pcpp::IPv4Network netTo(right);

    if (netFrom.getPrefixLen() != netTo.getPrefixLen())
        return EAppError::InvalidPrefixLengthMismatch;

    m_mapV4.emplace_back(netFrom, netTo);

    return std::error_code{};
}

std::error_code CCommandLineArguments::parse_addr_remap_v6(const std::string_view svAddrEntryV6)
{
    // Ensure its in format '[]:[]'
    if (!svAddrEntryV6.ends_with(']'))
        return EAppError::InvalidIPv6Pair;

    const size_t nDelim = svAddrEntryV6.find("]:[");
    if (std::string_view::npos == nDelim)
        return EAppError::InvalidAddressDelimiter;

    const std::string left(svAddrEntryV6.begin() + 1, svAddrEntryV6.begin() + nDelim);
    const std::string right(svAddrEntryV6.begin() + nDelim + 3, svAddrEntryV6.end() - 1);

    pcpp::IPv6Network netFrom(left);
    pcpp::IPv6Network netTo(right);

    if (netFrom.getPrefixLen() != netTo.getPrefixLen())
        return EAppError::InvalidPrefixLengthMismatch;

    m_mapV6.emplace_back(netFrom, netTo);

    return std::error_code{};
}

std::error_code CCommandLineArguments::parse(int argc, const char **argv)
{
    try
    {
        // Return non-exception errors as-is
        return parse_unsafe(argc, argv);
    }
    catch (cxxopts::exceptions::exception const &ex)
    {
        // cxxopts specific error, can be printed and treated as generic arguments error
        std::cerr << ex.what() << std::endl;
        return EAppError::GenericArgumentsError;
    }
    catch (std::system_error const &ex)
    {
        // Some error with a compatible error-code
        return ex.code();
    }
    catch (std::exception const &ex)
    {
        // Unknown error
        std::cerr << ex.what() << std::endl;
        return EAppError::Unknown;
    }
}

std::error_code CCommandLineArguments::parse_unsafe(int argc, const char **argv)
{
    bool bShowHelp = false;
    bool bShowVersion = false;
    bool bShowInterfaces = false;
    std::string strPortRemap;
    std::string strDestAddrRemap;

    static const char *szFilenameFlag = "file";

    // Define args
    cxxopts::Options options(version_name(), version_description());
    cxxopts::OptionAdder add = options.add_options();
    add(szFilenameFlag, "The input capture file path", cxxopts::value(m_strFilePath));
    add("r,portmap", "Rewrite port numbers", cxxopts::value(strPortRemap));
    add("D,dstipmap", "Rewrite destination IP addresses", cxxopts::value(strDestAddrRemap));
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
        return EAppError::ConflictOperations;

    // Check the modes used to print and exit
    if (bShowHelp)
    {
        m_op = ECommandLineOperation::HELP;
        m_strHelp = options.help();
        return {};
    }

    if (bShowVersion)
    {
        m_op = ECommandLineOperation::VERSION;
        return {};
    }

    if (bShowInterfaces)
    {
        m_op = ECommandLineOperation::LIST_INTERFACES;
        return {};
    }

    // Validate job arguments
    if (m_strFilePath.empty())
        return EAppError::MissingArgFile;

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
        return EAppError::ConflictLimit;
    else if (bLimitDuration)
        m_eLimit = ELimitMode::LIMIT_MAX_TIME;
    else if (bLimitPackets)
        m_eLimit = ELimitMode::LIMIT_MAX_PACKETS;

    // Parse the remaps
    if (std::error_code ec = parse_port_remap(strPortRemap))
        return ec;

    if (std::error_code ec = parse_dest_addr_remap(strDestAddrRemap))
        return ec;

    // The job is valid
    m_op = ECommandLineOperation::JOB;
    return {}; // No error
}

ECommandLineOperation CCommandLineArguments::get_operation() const
{
    return m_op;
}

std::string_view CCommandLineArguments::get_help() const
{
    return m_strHelp;
}

IJobArguments const &CCommandLineArguments::get_job_arguments() const
{
    if (m_op != ECommandLineOperation::JOB)
        throw std::system_error(EAppError::NoJob);
    return *this;
}

bool CCommandLineArguments::is_verbose() const
{
    return m_bVerbose;
}

std::string_view CCommandLineArguments::get_filename() const
{
    return CCommandLineArguments::m_strFilePath;
}

std::string_view CCommandLineArguments::get_interface_name() const
{
    return CCommandLineArguments::m_strInterface;
}

ERepeatMode CCommandLineArguments::get_repeat_mode() const
{
    return CCommandLineArguments::m_eRepeat;
}

unsigned int CCommandLineArguments::get_repeat_count() const
{
    if (m_eRepeat != ERepeatMode::REPEAT_TIMES)
        throw std::system_error(EAppError::UndefinedRepeatCount);
    return m_uRepeatTimes;
}

ELimitMode CCommandLineArguments::get_limit_mode() const
{
    return m_eLimit;
}

unsigned int CCommandLineArguments::get_limit_packets() const
{
    if (m_eLimit != ELimitMode::LIMIT_MAX_PACKETS)
        throw std::system_error(EAppError::UndefinedLimitPackets);
    return m_uLimitPackets;
}

unsigned int CCommandLineArguments::get_limit_duration() const
{
    if (m_eLimit != ELimitMode::LIMIT_MAX_TIME)
        throw std::system_error(EAppError::UndefinedLimitDuration);
    return m_uLimitTime;
}

ESpeedMode CCommandLineArguments::get_speed_mode() const
{
    return m_eSpeed;
}

double CCommandLineArguments::get_speed_multiplier() const
{
    if (m_eSpeed != ESpeedMode::SPEED_MULTIPLIER)
        throw std::system_error(EAppError::UndefinedSpeedMultiplier);
    return m_dSpeedMultiplier;
}

unsigned short CCommandLineArguments::get_port_remap(unsigned short usOriginalPort) const
{
    const auto it = m_mapPorts.find(usOriginalPort);
    if (it != m_mapPorts.cend())
        return it->second;

    // Return original port if not remapping is found
    return usOriginalPort;
}

pcpp::IPv4Address CCommandLineArguments::get_ipv4_destination_remap(pcpp::IPv4Address const &inputAddr) const
{
    // Find the remap entry containing this address
    auto it = std::find_if(
        m_mapV4.cbegin(), m_mapV4.cend(),
        [&inputAddr](IPv4Remap const &pair)
        { return pair.first.includes(inputAddr); });

    // Return same address if not found
    if (it == m_mapV4.cend())
        return inputAddr;

    // Found a subnet to remap
    pcpp::IPv4Network const &remapToNetwork = it->second;
    const uint8_t uPrefixLen = remapToNetwork.getPrefixLen();

    // Combine network prefix with varying host
    const uint32_t mask = 0xFFFFFFFF << (32 - uPrefixLen);

    const uint32_t prefix = pcpp::netToHost32(remapToNetwork.getNetworkPrefix().toInt()) & mask;
    const uint32_t host = pcpp::netToHost32(inputAddr.toInt()) & ~mask;

    return pcpp::IPv4Address(pcpp::hostToNet32(prefix | host));
}

pcpp::IPv6Address CCommandLineArguments::get_ipv6_destination_remap(pcpp::IPv6Address const &inputAddr) const
{
    // Find the remap entry containing this address
    auto it = std::find_if(
        m_mapV6.cbegin(), m_mapV6.cend(),
        [&inputAddr](IPv6Remap const &pair)
        { return pair.first.includes(inputAddr); });

    // Return same address if not found
    if (it == m_mapV6.cend())
        return inputAddr;

    // Found a network to remap
    pcpp::IPv6Network const &remapToNetwork = it->second;
    const auto prefixAddress = remapToNetwork.getNetworkPrefix();
    const uint8_t *pNetworkPrefix = prefixAddress.toBytes();
    const uint8_t *pInputAddr = inputAddr.toBytes();

    // The network prefix has a part with whole bytes then a split byte
    uint8_t uPrefix = remapToNetwork.getPrefixLen();
    const uint8_t uFullBytes = uPrefix / 8;
    const uint8_t uRemainingBits = uPrefix % 8;

    // Combine network prefix with varying host
    uint8_t i = 0;
    uint8_t result[16]{0x00};

    // Bytes fully defined by the network prefix
    for (; i < uFullBytes; ++i)
        result[i] = pNetworkPrefix[i];

    // Byte split between network prefix and host
    if (uRemainingBits > 0)
    {
        const uint8_t mask = 0xFF << (8 - uRemainingBits);
        result[i] =
            (pNetworkPrefix[uFullBytes] & mask) |
            (pInputAddr[uFullBytes] & ~mask);
        ++i;
    }

    // Bytes fully defined by the input
    for (; i < std::size(result); ++i)
        result[i] = pInputAddr[i];

    return pcpp::IPv6Address(result);
}
