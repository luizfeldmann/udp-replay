// Testing
#include <gtest/gtest.h>

// Local
#include "EAppError.h"
#include "CCommandLineArguments.h"

// Constants
static const char *g_szBinaryName = "udp-replay";
static const char *g_szInputFileName = "test-file.pcap";

// Test cases

TEST(cli, empty)
{
    static const char *argv[] = {g_szBinaryName};

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_EQ(ec, EAppError::MissingArgFile);
}

TEST(cli, help)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--help",
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::HELP);
}

TEST(cli, version)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--version",
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::VERSION);
}

TEST(cli, list_interfaces)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--interfaces",
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::LIST_INTERFACES);
}

TEST(cli, invalid_multiple_modes)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--help",
        "--version",
        "--interfaces",
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_EQ(ec, EAppError::ConflictOperations);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::INVALID);
}

TEST(cli, basic)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--intf1=eth0",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_job_arguments().get_filename(), g_szInputFileName);
    EXPECT_EQ(cli.get_job_arguments().get_interface_name(), "eth0");
}

TEST(cli, loop)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--loop=0",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_repeat_mode(), ERepeatMode::LOOP);
}

TEST(cli, repeat)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--loop=123",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_repeat_mode(), ERepeatMode::REPEAT_TIMES);
    EXPECT_EQ(cli.get_job_arguments().get_repeat_count(), 123);
}

TEST(cli, multiplier)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--multiplier=1.5",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_speed_mode(), ESpeedMode::SPEED_MULTIPLIER);
    EXPECT_EQ(cli.get_job_arguments().get_speed_multiplier(), 1.5);
}

TEST(cli, limit_duration)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--duration=60",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_limit_mode(), ELimitMode::LIMIT_MAX_TIME);
    EXPECT_EQ(cli.get_job_arguments().get_limit_duration(), 60);
}

TEST(cli, limit_packets)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--limit=1000",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_limit_mode(), ELimitMode::LIMIT_MAX_PACKETS);
    EXPECT_EQ(cli.get_job_arguments().get_limit_packets(), 1000);
}

TEST(cli, invalid_multiple_limits)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--limit=1000",
        "--duration=60",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_EQ(ec, EAppError::ConflictLimit);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::INVALID);
}

TEST(cli, portremap_single)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--portmap=80:8080",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_port_remap(80), 8080);
}

TEST(cli, portremap_multi)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--portmap=80:8080,443:8443",
        g_szInputFileName,
    };
    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_job_arguments().get_port_remap(80), 8080);
    EXPECT_EQ(cli.get_job_arguments().get_port_remap(443), 8443);
}

TEST(cli, portremap_range_single)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--portmap=8080-8090:3000",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_job_arguments().get_port_remap(8080), 3000);
    EXPECT_EQ(cli.get_job_arguments().get_port_remap(8085), 3000);
    EXPECT_EQ(cli.get_job_arguments().get_port_remap(8090), 3000);
}

TEST(cli, destremap_v4_single)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--dstipmap=192.168.0.0/16:10.17.0.0/16",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_ipv4_destination_remap(
                  pcpp::IPv4Address("192.168.10.20")),
              pcpp::IPv4Address("10.17.10.20"));
}

TEST(cli, destremap_v4_multi)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--dstipmap=192.168.56.0/24:10.17.99.0/24,172.16.10.0/24:172.31.55.0/24",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_ipv4_destination_remap(
                  pcpp::IPv4Address("192.168.56.123")),
              pcpp::IPv4Address("10.17.99.123"));
    EXPECT_EQ(cli.get_job_arguments().get_ipv4_destination_remap(
                  pcpp::IPv4Address("172.16.10.69")),
              pcpp::IPv4Address("172.31.55.69"));
}

TEST(cli, mismatch_prefixes)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--dstipmap=1.2.3.4/8:5.6.7.8/16",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_EQ(ec, EAppError::InvalidPrefixLengthMismatch);
}

TEST(cli, destremap_v6_64)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--dstipmap=[2001:db8::/64]:[fd00:beef::/64]",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_ipv6_destination_remap(
                  pcpp::IPv6Address("2001:db8::5678:9abc:def0:1111")),
              pcpp::IPv6Address("fd00:beef::5678:9abc:def0:1111"));
}

TEST(cli, destremap_v6_36)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--dstipmap=[2001:db8::/36]:[3001:dea::/36]",
        g_szInputFileName,
    };

    CCommandLineArguments cli;
    std::error_code ec = cli.parse(std::size(argv), argv);

    EXPECT_FALSE(ec);
    EXPECT_EQ(cli.get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(cli.get_job_arguments().get_ipv6_destination_remap(
                  pcpp::IPv6Address("2001:db8:0bcd:1234:5678::1")),
              pcpp::IPv6Address("3001:dea:0bcd:1234:5678::1"));
}
