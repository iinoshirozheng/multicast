#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace stream_buffer
{
    namespace common
    {

        // Constants namespace to avoid macro pollution
        namespace constants
        {
            // Buffer sizes
            constexpr int MEGA_BYTE = 1048576;
            constexpr int DEFAULT_BUFFER_SIZE = 80;

            // Return codes
            constexpr int JOIN_FAILED = -1;
            constexpr int JOIN_SUCCEED = 0;
            constexpr int DECODE_BCD_FAILED = -1;
            constexpr int PROCESS_FAILED = -1;

            // Socket options
            constexpr int REUSE_ADDR = 1;
            constexpr int MCAST_ALL = 0;
        }

        // Type definitions
        using Byte = uint8_t;
        using u8 = uint8_t;
        using u16 = uint16_t;
        using u32 = uint32_t;
        using u64 = uint64_t;
        using i8 = int8_t;
        using i16 = int16_t;
        using i32 = int32_t;
        using i64 = int64_t;

        // Socket types as enum classes for type safety
        enum class SocketDomain
        {
            IPV4 = AF_INET,
            IPV6 = AF_INET6
        };

        enum class SocketType
        {
            TCP = SOCK_STREAM,
            UDP = SOCK_DGRAM
        };

        // Configuration class for multicast settings
        class MulticastConfig
        {
        public:
            SocketDomain domain;
            SocketType type;
            int protocol;
            std::string group_ip;
            int port;
            std::string interface_name;
            std::string interface_ip;
            int recv_buffer_size;

            MulticastConfig(
                SocketDomain domain = SocketDomain::IPV4,
                SocketType type = SocketType::UDP,
                int protocol = 0,
                const std::string &group_ip = "",
                int port = 0,
                const std::string &interface_name = "",
                const std::string &interface_ip = "",
                int recv_buffer_size = 0) : domain(domain),
                                            type(type),
                                            protocol(protocol),
                                            group_ip(group_ip),
                                            port(port),
                                            interface_name(interface_name),
                                            interface_ip(interface_ip),
                                            recv_buffer_size(recv_buffer_size) {}
        };

        // Return codes
        enum class Status
        {
            OK = 0,
            ERROR = -1,
            TIMEOUT = -2,
            INVALID_PARAM = -3,
            NO_DATA = -4,
            BUFFER_FULL = -5,
        };

    } // namespace common
} // namespace stream_buffer