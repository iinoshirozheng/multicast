#pragma once

#include "common/types.h"
#include <string>
#include <cstddef>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace stream_buffer
{
    namespace network
    {
        /**
         * @brief Create a multicast socket
         *
         * @param config Multicast configuration
         * @return int Socket file descriptor or -1 on error
         */
        int CreateSocket(const common::MulticastConfig &config);

        /**
         * @brief Join a multicast group
         *
         * @param socket_fd Socket file descriptor
         * @param group_ip Multicast group IP address
         * @param port Port number
         * @param interface_name Network interface name (optional)
         * @param interface_ip Network interface IP address
         * @return int 0 on success, -1 on error
         */
        int JoinMulticastGroup(
            int socket_fd,
            const std::string &group_ip,
            int port,
            const std::string &interface_name,
            const std::string &interface_ip);

        /**
         * @brief Leave a multicast group
         *
         * @param socket_fd Socket file descriptor
         * @param group_ip Multicast group IP address
         * @param interface_ip Network interface IP address
         * @return int 0 on success, -1 on error
         */
        int LeaveMulticastGroup(
            int socket_fd,
            const std::string &group_ip,
            const std::string &interface_ip);

        /**
         * @brief Network receiver interface
         */
        class INetworkReceiver
        {
        public:
            virtual ~INetworkReceiver() = default;

            /**
             * @brief Receive data from the network
             *
             * @param buffer Buffer to store received data
             * @param buffer_size Size of the buffer
             * @return int Bytes received or -1 on error
             */
            virtual int ReceiveData(char *buffer, size_t buffer_size) = 0;
        };

        /**
         * @brief Multicast receiver implementation
         */
        class MulticastReceiver : public INetworkReceiver
        {
        public:
            /**
             * @brief Construct a new Multicast Receiver
             *
             * @param socket_fd Socket file descriptor
             */
            explicit MulticastReceiver(int socket_fd);

            /**
             * @brief Receive data from multicast group
             *
             * @param buffer Buffer to store received data
             * @param buffer_size Size of the buffer
             * @return int Bytes received or -1 on error
             */
            int ReceiveData(char *buffer, size_t buffer_size) override;

            /**
             * @brief Get the source IP address of the last received packet
             *
             * @return Source IP address as string
             */
            std::string GetSourceIP() const;

            /**
             * @brief Get the source port of the last received packet
             *
             * @return Source port number
             */
            int GetSourcePort() const;

        private:
            int socket_fd_;
            struct sockaddr_in src_addr_;
            socklen_t addr_len_;
        };
    } // namespace network
} // namespace stream_buffer