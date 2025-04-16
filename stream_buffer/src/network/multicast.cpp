#include "network/multicast.h"
#include "utils/debug.h"
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cerrno>

namespace stream_buffer
{
    namespace network
    {
        namespace
        {
            // Helper functions for multicast socket configuration
            bool SetReuseAddress(int socket_id)
            {
                const int reuse_addr = common::constants::REUSE_ADDR;
                if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR,
                               &reuse_addr, sizeof(reuse_addr)) < 0)
                {
                    FMT_PRINT("Failed to set SO_REUSEADDR: %s\n", strerror(errno));
                    return false;
                }
                return true;
            }

            bool SetReceiveBuffer(int socket_id)
            {
                const int buffer_size = common::constants::MEGA_BYTE;
                if (setsockopt(socket_id, SOL_SOCKET, SO_RCVBUF,
                               &buffer_size, sizeof(buffer_size)) < 0)
                {
                    FMT_PRINT("Failed to set SO_RCVBUF: %s\n", strerror(errno));
                    return false;
                }
                return true;
            }

            bool JoinMulticastGroupInternal(int socket_id, const char *group_ip, const char *if_ip)
            {
                if (!group_ip || group_ip[0] == '\0')
                {
                    FMT_PRINT("Invalid multicast IP\n");
                    return false;
                }

                struct ip_mreq mreq = {};
                mreq.imr_multiaddr.s_addr = inet_addr(group_ip);
                mreq.imr_interface.s_addr = inet_addr(if_ip);

                if (setsockopt(socket_id, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                               &mreq, sizeof(mreq)) < 0)
                {
                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(mreq.imr_multiaddr), ip_str, INET_ADDRSTRLEN);
                    FMT_PRINT("Failed to join multicast group %s: %s\n",
                              ip_str, strerror(errno));
                    return false;
                }
                return true;
            }

            bool SetMulticastLoopback(int socket_id)
            {
                const int mcast_setting = common::constants::MCAST_ALL;
                if (setsockopt(socket_id, IPPROTO_IP, IP_MULTICAST_LOOP,
                               &mcast_setting, sizeof(mcast_setting)) < 0)
                {
                    FMT_PRINT("Failed to set IP_MULTICAST_LOOP: %s\n", strerror(errno));
                    return false;
                }
                return true;
            }

            bool BindSocket(int socket_id, int port)
            {
                struct sockaddr_in server_addr = {};
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(port);
                server_addr.sin_addr.s_addr = INADDR_ANY;

                if (bind(socket_id, reinterpret_cast<sockaddr *>(&server_addr),
                         sizeof(server_addr)) < 0)
                {
                    FMT_PRINT("Failed to bind socket to port %d: %s\n",
                              port, strerror(errno));
                    return false;
                }
                return true;
            }
        } // anonymous namespace

        // Join multicast group and set up socket
        int JoinMulticastGroup(
            int socket_fd,
            const std::string &group_ip,
            int port,
            const std::string &interface_name,
            const std::string &interface_ip)
        {
            // Mark unused parameter to avoid compiler warning
            (void)interface_name;

            // Set socket options for multicast reception
            if (!SetReuseAddress(socket_fd) ||
                !SetReceiveBuffer(socket_fd) ||
                !JoinMulticastGroupInternal(socket_fd, group_ip.c_str(), interface_ip.c_str()) ||
                !SetMulticastLoopback(socket_fd) ||
                !BindSocket(socket_fd, port))
            {
                close(socket_fd);
                return common::constants::JOIN_FAILED;
            }

            return common::constants::JOIN_SUCCEED;
        }

        // Create socket for network configuration
        int CreateSocket(const common::MulticastConfig &config)
        {
            // Create UDP socket
            int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (socket_fd < 0)
            {
                FMT_PRINT("Failed to create socket: %s\n", strerror(errno));
                return -1;
            }

            // Set socket options
            int reuse = 1;
            if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
            {
                FMT_PRINT("Failed to set socket option SO_REUSEADDR: %s\n", strerror(errno));
                close(socket_fd);
                return -1;
            }

#ifdef SO_REUSEPORT
            // Set SO_REUSEPORT if available
            if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
            {
                FMT_PRINT("Error setting SO_REUSEPORT: %s\n", strerror(errno));
                // Not critical, continue
            }
#endif

            // Set recv buffer size if specified
            if (config.recv_buffer_size > 0)
            {
                int buffer_size = config.recv_buffer_size;
                if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF,
                               &buffer_size, sizeof(buffer_size)) < 0)
                {
                    FMT_PRINT("Failed to set socket option SO_RCVBUF: %s\n", strerror(errno));
                    close(socket_fd);
                    return -1;
                }
            }

            // Bind socket to address and port
            struct sockaddr_in addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(config.port);

            if (bind(socket_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
            {
                FMT_PRINT("Failed to bind socket: %s\n", strerror(errno));
                close(socket_fd);
                return -1;
            }

            return socket_fd;
        }

        // MulticastReceiver implementation
        MulticastReceiver::MulticastReceiver(int socket_fd)
            : socket_fd_(socket_fd), addr_len_(sizeof(src_addr_))
        {
            std::memset(&src_addr_, 0, sizeof(src_addr_));
        }

        int MulticastReceiver::ReceiveData(char *buffer, size_t buffer_size)
        {
            if (!buffer || buffer_size == 0)
            {
                FMT_PRINT("Invalid buffer or buffer size\n");
                return -1;
            }

            if (socket_fd_ < 0)
            {
                FMT_PRINT("Invalid socket descriptor\n");
                return -1;
            }

            // Reset address length before each receive
            addr_len_ = sizeof(src_addr_);

            // Receive data
            int bytes_received = recvfrom(socket_fd_, buffer, buffer_size, 0,
                                          reinterpret_cast<struct sockaddr *>(&src_addr_),
                                          &addr_len_);

            if (bytes_received < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // Non-blocking socket timeout, not an error
                    return 0;
                }
                else if (errno == EINTR)
                {
                    // Interrupted by signal, not an error
                    FMT_PRINT("Receive interrupted by signal\n");
                    return 0;
                }
                else
                {
                    // Real error
                    FMT_PRINT("Failed to receive data: %s\n", strerror(errno));
                    return -1;
                }
            }

            if (bytes_received > 0 && bytes_received <= static_cast<int>(buffer_size))
            {
                FMT_PRINT("Received %d bytes from %s:%d\n",
                          bytes_received, GetSourceIP().c_str(), GetSourcePort());
            }

            return bytes_received;
        }

        std::string MulticastReceiver::GetSourceIP() const
        {
            char ip_str[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &(src_addr_.sin_addr), ip_str, INET_ADDRSTRLEN) == nullptr)
            {
                return "unknown";
            }
            return std::string(ip_str);
        }

        int MulticastReceiver::GetSourcePort() const
        {
            return ntohs(src_addr_.sin_port);
        }

        int LeaveMulticastGroup(int socket_fd, const std::string &group_ip, const std::string &interface_ip)
        {
            if (socket_fd < 0)
            {
                FMT_PRINT("Invalid socket descriptor for leaving multicast group\n");
                return -1;
            }

            struct ip_mreq mreq{};

            // Set the multicast address to leave
            if (inet_pton(AF_INET, group_ip.c_str(), &mreq.imr_multiaddr) <= 0)
            {
                FMT_PRINT("Invalid multicast address to leave: %s\n", group_ip.c_str());
                return -1;
            }

            // Set the local interface
            if (!interface_ip.empty())
            {
                if (inet_pton(AF_INET, interface_ip.c_str(), &mreq.imr_interface) <= 0)
                {
                    FMT_PRINT("Invalid interface address: %s\n", interface_ip.c_str());
                    return -1;
                }
            }
            else
            {
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            }

            // Leave the multicast group
            if (setsockopt(socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
            {
                FMT_PRINT("Failed to leave multicast group %s: %s\n",
                          group_ip.c_str(), strerror(errno));
                return -1;
            }

            FMT_PRINT("Successfully left multicast group %s\n", group_ip.c_str());
            return 0;
        }

    } // namespace network
} // namespace stream_buffer