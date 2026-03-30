// Testing
#include <gtest/gtest.h>

// Local
#include "parsecli.h"

// Constants
static const char *g_szBinaryName = "udp-replay";
static const char *g_szInputFileName = "test-file.pcap";

// Test cases

TEST(cli, empty)
{
    static const char *argv[] = {g_szBinaryName};
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::INVALID);
}

TEST(cli, help)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--help",
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::HELP);
}

TEST(cli, version)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--version",
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::VERSION);
}

TEST(cli, list_interfaces)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--interfaces",
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::LIST_INTERFACES);
}

TEST(cli, invalid_multiple_modes)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--help",
        "--version",
        "--interfaces",
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::INVALID);
}

TEST(cli, loop)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--loop=0",
        g_szInputFileName,
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_repeat_mode(), ERepeatMode::LOOP);
}

TEST(cli, repeat)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--loop=123",
        g_szInputFileName,
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_repeat_mode(), ERepeatMode::REPEAT_TIMES);
    EXPECT_EQ(pCli->get_job_arguments().get_repeat_count(), 123);
}

TEST(cli, multiplier)
{
    static const char *argv[] = {g_szBinaryName, "--multiplier=1.5", g_szInputFileName};
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_speed_mode(), ESpeedMode::SPEED_MULTIPLIER);
    EXPECT_EQ(pCli->get_job_arguments().get_speed_multiplier(), 1.5);
}

TEST(cli, limit_duration)
{
    static const char *argv[] = {g_szBinaryName, "--duration=60", g_szInputFileName};
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_limit_mode(), ELimitMode::LIMIT_MAX_TIME);
    EXPECT_EQ(pCli->get_job_arguments().get_limit_duration(), 60);
}

TEST(cli, limit_packets)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--limit=1000",
        g_szInputFileName,
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_limit_mode(), ELimitMode::LIMIT_MAX_PACKETS);
    EXPECT_EQ(pCli->get_job_arguments().get_limit_packets(), 1000);
}

TEST(cli, invalid_multiple_limits)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--limit=1000",
        "--duration=60",
        g_szInputFileName,
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::INVALID);
}

TEST(cli, portremap_single)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--portmap=80:8080",
        g_szInputFileName,
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_port_remap(80), 8080);
}

TEST(cli, portremap_multi)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--portmap=80:8080,443:8443",
        g_szInputFileName,
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_port_remap(80), 8080);
    EXPECT_EQ(pCli->get_job_arguments().get_port_remap(443), 8443);
}

TEST(cli, portremap_range_single)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--portmap=8080-8090:3000",
        g_szInputFileName,
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_port_remap(8080), 3000);
    EXPECT_EQ(pCli->get_job_arguments().get_port_remap(8085), 3000);
    EXPECT_EQ(pCli->get_job_arguments().get_port_remap(8090), 3000);
}

TEST(cli, destremap_v4_single)
{
    static const char *argv[] = {
        g_szBinaryName,
        "--dstipmap=192.168.0.0/16:10.17.0.0/16",
        g_szInputFileName,
    };
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_ipv4_destination_remap(
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
    auto pCli = parse_cli(std::size(argv), argv);

    EXPECT_EQ(pCli->get_operation(), ECommandLineOperation::JOB);
    EXPECT_EQ(pCli->get_job_arguments().get_ipv4_destination_remap(
                  pcpp::IPv4Address("192.168.56.123")),
              pcpp::IPv4Address("10.17.99.123"));
    EXPECT_EQ(pCli->get_job_arguments().get_ipv4_destination_remap(
                  pcpp::IPv4Address("172.16.10.69")),
              pcpp::IPv4Address("172.31.55.69"));
}
