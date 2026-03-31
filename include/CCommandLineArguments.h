#ifndef _C_COMMAND_LINE_ARGUMENTS_H_
#define _C_COMMAND_LINE_ARGUMENTS_H_

// Local
#include "ICommandLineArguments.h"

// STD
#include <memory>
#include <unordered_map>
#include <vector>

//! Command line arguments accessor
class CCommandLineArguments
    : public ICommandLineArguments,
      private IJobArguments
{
private:
    //! Entry mapping an between IPv4 subnets
    using IPv4Remap = std::pair<pcpp::IPv4Network, pcpp::IPv4Network>;

    //! Entry mapping an between IPv6 subnets
    using IPv6Remap = std::pair<pcpp::IPv6Network, pcpp::IPv6Network>;

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

    //! Map between V4 subnets
    std::vector<IPv4Remap> m_mapV4;

    //! Map between V6 subnets
    std::vector<IPv6Remap> m_mapV6;

    //! Parses the port remap argument
    std::error_code parse_port_remap(std::string_view svPortRemap);

    //! Parse the address remap argument
    std::error_code parse_dest_addr_remap(const std::string_view);

    //! Parses a single pair of v4 addresses
    std::error_code parse_addr_remap_v4(const std::string_view);

    //! Parses a single pair of v6 addresses
    std::error_code parse_addr_remap_v6(const std::string_view);

    //! Unsafe parsing of arguments
    std::error_code parse_unsafe(int argc, const char **argv);

public:
    //! Constructor
    CCommandLineArguments();

    //! Parses the command line arguments
    std::error_code parse(int argc, const char **argv);

    //! Overrides from ICommandLineArguments
    //! @{
    ECommandLineOperation get_operation() const override;
    std::string_view get_help() const override;
    IJobArguments const &get_job_arguments() const override;
    //! @}

    //! Overrides from IJobArguments
    //! @{
    bool is_verbose() const override;
    std::string_view get_filename() const override;
    std::string_view get_interface_name() const override;
    ERepeatMode get_repeat_mode() const override;
    unsigned int get_repeat_count() const override;
    ELimitMode get_limit_mode() const override;
    unsigned int get_limit_packets() const override;
    unsigned int get_limit_duration() const override;
    ESpeedMode get_speed_mode() const override;
    double get_speed_multiplier() const override;
    unsigned short get_port_remap(unsigned short usOriginalPort) const override;
    pcpp::IPv4Address get_ipv4_destination_remap(pcpp::IPv4Address const &inputAddr) const override;
    pcpp::IPv6Address get_ipv6_destination_remap(pcpp::IPv6Address const &inputAddr) const override;
    //! @}
};

#endif // _C_COMMAND_LINE_ARGUMENTS_H_
