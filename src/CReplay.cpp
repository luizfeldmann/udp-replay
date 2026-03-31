// Local
#include "CReplay.h"
#include "EAppError.h"

// Dependencies
#include <pcapplusplus/PcapLiveDeviceList.h>
#include <pcapplusplus/Packet.h>
#include <pcapplusplus/UdpLayer.h>
#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/IPv6Layer.h>

#include <boost/asio/post.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/beast/core/bind_handler.hpp>

// STD
#include <iostream>
#include <condition_variable>

void CReplay::print_interfaces()
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

CReplay::CReplay(IJobArguments const &args)
    : m_args(args), m_io(), m_timer(m_io)
{
}

std::error_code CReplay::run_replay()
{
    try
    {
        // Pass non-exception errors as-is
        return run_replay_unsafe();
    }
    catch (std::system_error const &ex)
    {
        // Pass out the error code of the exceptions
        return ex.code();
    }
    catch (std::exception const &ex)
    {
        // Print error message and pass out unknown error code
        std::cerr << ex.what() << std::endl;
        return EAppError::Unknown;
    }
}

std::error_code CReplay::run_replay_unsafe()
{
    // Open the replay file
    std::string strFilename(m_args.get_filename());
    if (strFilename.ends_with(".pcap"))
        m_pReader = std::make_unique<pcpp::PcapFileReaderDevice>(strFilename);
    else if (strFilename.ends_with(".pcapng"))
        m_pReader = std::make_unique<pcpp::PcapNgFileReaderDevice>(strFilename);
    else
        return EAppError::UnsupportedFileType;

    // Handle error opening the file
    if (!m_pReader->open())
        return EAppError::FileNotFound;

    // Register for signals to stop the job
    boost::asio::signal_set signals(m_io, SIGINT, SIGTERM);
    signals.async_wait(
        boost::asio::bind_cancellation_slot(m_cancel_signal.slot(),
                                            boost::beast::bind_front_handler(&CReplay::on_signal, this)));

    // Schedules the first packet
    m_start_time = std::chrono::steady_clock::now();
    m_next_packet_time = m_start_time;
    schedule_next_packet();

    // Run all async operations until completion
    // Bubbles out the exceptions from the async operations
    m_io.run();

    // No error
    return std::error_code{};
}

void CReplay::on_signal(boost::system::error_code const &ec, int sig)
{
    // When receiving SIGINT, SIGTERM, etc... just cancel all operations
    m_cancel_signal.emit(boost::asio::cancellation_type::all);
}

bool CReplay::get_next_packet(pcpp::RawPacket &rawPacket)
{
    // check for limitations
    switch (m_args.get_limit_mode())
    {
    case ELimitMode::LIMIT_MAX_PACKETS:
        // Check if we already sent the limited number of packets
        if (m_count_sent_total >= m_args.get_limit_packets())
            return false;
        break;

    case ELimitMode::LIMIT_MAX_TIME:
        // Check if time limitation has elapsed
        const auto now = std::chrono::steady_clock::now();
        if (now > m_start_time + std::chrono::seconds(m_args.get_limit_duration()))
            return false;
        break;
    }

    // simply read the next packet in the queue
    if (m_pReader->getNextPacket(rawPacket))
        return true;

    // no next packet exists, we reached end of file
    // count how many times we completed the full file
    ++m_count_completions;

    // decide if continue looping according to job arguments
    switch (m_args.get_repeat_mode())
    {
    case ERepeatMode::REPEAT_TIMES:
        if (m_count_completions >= m_args.get_repeat_count())
            return false; // already repeated the specified number of times
        [[fallthrough]];  // go to the loop case

    case ERepeatMode::LOOP:
        // close and re-open to go back to start of file
        m_pReader->close();
        if (!m_pReader->open())
            throw std::system_error(EAppError::FileNotFound);
        // reset the per-loop metrics
        m_count_sent_loop = 0;
        // read first packet again
        return m_pReader->getNextPacket(rawPacket);
    }

    // no repeat, this really is the end
    return false;
}

void CReplay::schedule_next_packet()
{
    // get the next packet to schedule, if it exists
    pcpp::RawPacket rawPacket;
    if (!get_next_packet(rawPacket))
    {
        // no more work, cancel the awaiter for the interrupt signals
        return m_cancel_signal.emit(boost::asio::cancellation_type::all);
    }

    // Absolute time of the current packet
    const timespec tsCurrent = rawPacket.getPacketTimeStamp();

    // There is no prev time for the first packet or when restarting on a loop
    if (m_count_sent_loop)
        m_tsPrevPacket = tsCurrent;

    // calculate the time delta to the current packet
    std::chrono::steady_clock::duration deltaTime =
        std::chrono::seconds(tsCurrent.tv_sec - m_tsPrevPacket.tv_sec) +
        std::chrono::nanoseconds(tsCurrent.tv_nsec - m_tsPrevPacket.tv_nsec);

    // apply speed multiplier factor
    if (m_args.get_speed_mode() == ESpeedMode::SPEED_MULTIPLIER)
    {
        deltaTime /= m_args.get_speed_multiplier();
    }

    // monotonically increment the presentation time
    m_next_packet_time += deltaTime;

    // schedule the event to send the next packet
    m_timer.expires_at(m_next_packet_time);
    m_timer.async_wait(
        boost::asio::bind_cancellation_slot(m_cancel_signal.slot(),
                                            boost::beast::bind_front_handler(&CReplay::on_send_packet, this, rawPacket)));
}

void CReplay::on_send_packet(pcpp::RawPacket rawPacket, boost::system::error_code const &ec)
{
    // handle interrupted by user
    if (ec == boost::asio::error::operation_aborted)
        return;

    // propagate unexpected errors to the caller
    if (ec)
        throw std::system_error(ec);

    // parse the packet
    pcpp::Packet packet(&rawPacket);

    auto const pIp4 = packet.getLayerOfType<pcpp::IPv4Layer>();
    auto const pIp6 = packet.getLayerOfType<pcpp::IPv6Layer>();
    auto const pUdp = packet.getLayerOfType<pcpp::UdpLayer>();

    // check if the packet has the required protocols
    if (!pUdp || (!pIp4 && !pIp6))
    {
        // continue to next packet without sending this one
        boost::asio::post(m_io, boost::beast::bind_front_handler(&CReplay::schedule_next_packet, this));
    }

    // perform the rewrites in ports and addresses
    unsigned short usDestPort = m_args.get_port_remap(pUdp->getDstPort());

    if (pIp4)
        m_args.get_ipv4_destination_remap(pIp4->getDstIPv4Address());
    else
        m_args.get_ipv6_destination_remap(pIp6->getDstIPv6Address());
}
