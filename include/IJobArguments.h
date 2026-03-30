#ifndef _I_JOB_ARGUMENTS_H_
#define _I_JOB_ARGUMENTS_H_

// Local
#include "ERepeatMode.h"
#include "ELimitMode.h"
#include "ESpeedMode.h"

// Dependencies
#include <pcapplusplus/IpAddress.h>

// STD
#include <string_view>

//! Describes all the arguments required for a replay job
class IJobArguments
{
public:
    virtual ~IJobArguments() = default;

    //! Reads the verbose flag
    virtual bool is_verbose() const = 0;

    //! Reads the name of the input file.
    virtual std::string_view get_filename() const = 0;

    //! Reads the name of the replay interface.
    virtual std::string_view get_interface_name() const = 0;

    //! Gets the repetition mode
    virtual ERepeatMode get_repeat_mode() const = 0;

    //! Reads the number of times to repeat this job.
    //! @throw If the repeat mode is not ERepeatMode::REPEAT_TIMES
    virtual unsigned int get_repeat_count() const = 0;

    //! Gets the job limiting mode.
    virtual ELimitMode get_limit_mode() const = 0;

    //! Gets the limited maximum number of packets.
    //! @throw If the limit mode is not ELimitMode::LIMIT_MAX_PACKETS
    virtual unsigned int get_limit_packets() const = 0;

    //! Gets the limited duration of the job.
    //! @throw If the limit mode is not ELimitMode::LIMIT_MAX_TIME
    virtual unsigned int get_limit_duration() const = 0;

    //! Gets the speed mode of the job
    virtual ESpeedMode get_speed_mode() const = 0;

    //! Get the speed multiplier factor.
    //! @throw If the speed mode is not ESpeedMode::SPEED_MULTIPLIER
    virtual double get_speed_multiplier() const = 0;

    //! Gets the remapped for number from the given original port number.
    //! If no remapping is defined, returns the same value as the input argument.
    virtual unsigned short get_port_remap(unsigned short usOriginalPort) const = 0;

    //! Gets the remap of a destination IPv4 address
    virtual pcpp::IPv4Address get_ipv4_destination_remap(pcpp::IPv4Address const &dest_ipv4) const = 0;

    //! Gets the remap of a destination IPv6 address
    virtual pcpp::IPv6Address get_ipv6_destination_remap(pcpp::IPv6Address const &dest_ipv6) const = 0;
};

#endif // _I_JOB_ARGUMENTS_H_
