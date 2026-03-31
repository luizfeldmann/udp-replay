#ifndef _C_REPLAY_H_
#define _C_REPLAY_H_

// Local
#include "IJobArguments.h"

// Dependencies
#include <pcapplusplus/PcapFileDevice.h>

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

    //! Gets the next packet
    //! Handles looping and limiting
    bool get_next_packet(pcpp::RawPacket &);

    //! Schedules the next packet for transmitting
    void schedule_next_packet();

    //! Callback for when a packet is ready to be sent
    void on_send_packet(pcpp::RawPacket rawPacket, boost::system::error_code const &ec);

    //! Callback for when a signal is emitted
    void on_signal(boost::system::error_code const &ec, int sig);

    //! Throwing runner for the job
    std::error_code run_replay_unsafe();

public:
    //! Constructor
    CReplay(IJobArguments const &);

    //! Executes the replay job
    std::error_code run_replay();

    //! Prints the network interfaces
    static void print_interfaces();
};

#endif // _C_REPLAY_H_
