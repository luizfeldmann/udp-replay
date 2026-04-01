#ifndef _C_REPLAY_H_
#define _C_REPLAY_H_

// Local
#include "IJobArguments.h"

// Dependencies
#include <pcapplusplus/PcapFileDevice.h>

#include <boost/asio/ip/udp.hpp>
#include <boost/unordered_map.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/steady_timer.hpp>

//! Handles a replay job
class CReplay
{
private:
    //! Reference to the replay job arguments
    IJobArguments const &m_args;

    //! Context for the async operations
    boost::asio::io_context m_io;

    //! Signals the cancellation of the job execution
    boost::asio::cancellation_signal m_cancel_signal;

    //! Scheduler for packet timing
    boost::asio::steady_timer m_timer;

    //! Reader for the input capture file
    std::unique_ptr<pcpp::IFileReaderDevice> m_pReader;

    //! Counts the total of packets sent
    size_t m_count_sent_total = 0;

    //! Counts the number of packets sent in the current loop
    size_t m_count_sent_loop = 0;

    //! Counts how many times the full file has been completed
    size_t m_count_completions = 0;

    //! Timestamp of the previous packet, for time delta tracking
    timespec m_tsPrevPacket = {0, 0};

    //! The time when the job started
    std::chrono::steady_clock::time_point m_start_time;

    //! The time to present the next packet
    std::chrono::steady_clock::time_point m_next_packet_time;

    //! Key to identify a socket in the pool
    struct socket_key
    {
        //! protocol version
        enum class protocol_version : uint8_t
        {
            ipv4 = 4,
            ipv6 = 6
        } m_ver;

        //! cast type
        enum class cast_type : uint8_t
        {
            unicast,
            multicast
        } m_cast;

        //! constructor
        socket_key(protocol_version, cast_type);

        //! Comparator
        bool operator==(socket_key const &) const noexcept;

        //! Hasher
        size_t hash() const noexcept;
    };

    //! Helper functor to hash the socket key
    struct socket_key_hasher
    {
        //! Hashes the socket key
        std::size_t operator()(CReplay::socket_key const &) const noexcept;
    };

    //! Collection of allocated sockets
    boost::unordered_map<socket_key, boost::asio::ip::udp::socket, socket_key_hasher> m_socket_pool;

    // Buffer of TX data
    std::vector<uint8_t> m_send_buf;

    //! Gets the address associated to the v4 network interface
    static boost::asio::ip::address_v4 get_interface_address(std::string_view);

    //! Gets the index of the v6 interface
    static unsigned int get_interface_index(std::string_view);

    //! Finds an existing socket in the pool or creates it
    boost::asio::ip::udp::socket &find_or_create_socket(boost::asio::ip::udp::endpoint const &);

    //! Gets the next packet
    //! Handles looping and limiting
    bool get_next_packet(pcpp::RawPacket &);

    //! Schedules the next packet for transmitting
    void schedule_next_packet();

    //! Callback for when a packet is ready to be sent
    void on_send_packet(pcpp::RawPacket, boost::system::error_code const &);

    //! Callback after a packet sending is complete
    void on_sent_complete(boost::system::error_code const &, size_t);

    //! Callback for when a signal is emitted
    void on_signal(boost::system::error_code const &, int);

    //! Throwing runner for the job
    std::error_code run_replay_unsafe();

public:
    //! Constructor
    CReplay(IJobArguments const &);

    //! Destructor
    ~CReplay();

    //! Executes the replay job
    std::error_code run_replay();

    //! Prints the network interfaces
    static void print_interfaces();
};

#endif // _C_REPLAY_H_
