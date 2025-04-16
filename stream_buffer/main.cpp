#include "core/buffer_processor.h"
#include "common/types.h"
#include <string>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>

using namespace stream_buffer;

// Print usage information
void printUsage(const char *programName)
{
    std::cerr << "Usage: " << programName << " [options]\n\n"
              << "Options:\n"
              << "  -g <group_ip>      Set multicast group IP address (required if not using -j)\n"
              << "  -j <json_file>     Read configuration from JSON file\n"
              << "  -b <buffer_size>   Set buffer size in MB (default: 100)\n"
              << "  -p <port>          Set port number (default: 10000)\n"
              << "  -i <interface>     Set network interface (default: en049.135)\n"
              << "  -a <address>       Set local IP address (default: 10.71.205.68)\n"
              << "  -h                 Show this help message\n"
              << std::endl;
}

// Simple function to extract value from a JSON string
std::string extractJsonString(const std::string &json, const std::string &key)
{
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos)
        return "";

    // Find the first occurrence of ':' after the key
    pos = json.find(':', pos);
    if (pos == std::string::npos)
        return "";

    // Skip whitespace
    pos++;
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
        pos++;

    // Check if it's a string value (with quotes)
    if (pos < json.length() && json[pos] == '"')
    {
        size_t startPos = pos + 1;
        size_t endPos = json.find('"', startPos);
        if (endPos != std::string::npos)
            return json.substr(startPos, endPos - startPos);
    }
    // Check if it's a numeric value
    else if (pos < json.length() && (isdigit(json[pos]) || json[pos] == '-'))
    {
        size_t endPos = pos;
        while (endPos < json.length() && (isdigit(json[endPos]) || json[endPos] == '.'))
            endPos++;
        return json.substr(pos, endPos - pos);
    }

    return "";
}

// Load configuration from JSON file
bool loadConfigFromJson(const std::string &filename,
                        std::string &groupIp,
                        std::string &interface,
                        std::string &localIp,
                        int &port,
                        size_t &bufferSizeMB)
{
    try
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not open JSON file: " << filename << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string jsonContent = buffer.str();

        // Extract values from JSON with defaults if not present
        std::string value;

        value = extractJsonString(jsonContent, "group_ip");
        if (!value.empty())
            groupIp = value;

        value = extractJsonString(jsonContent, "interface");
        if (!value.empty())
            interface = value;

        value = extractJsonString(jsonContent, "local_ip");
        if (!value.empty())
            localIp = value;

        value = extractJsonString(jsonContent, "port");
        if (!value.empty())
            port = std::stoi(value);

        value = extractJsonString(jsonContent, "buffer_size_mb");
        if (!value.empty())
            bufferSizeMB = std::stoul(value);

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return false;
    }
}

// Parse command line arguments and create configuration
common::MulticastConfig ParseCommandLine(int argc, char *argv[], size_t &bufferSizeMB)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        exit(1);
    }

    // Set default values
    std::string groupIp = "225.0.0.1";
    std::string interface = "en049.135";
    std::string localIp = "10.71.205.68";
    int port = 10000;
    bufferSizeMB = 100;
    std::string jsonFile;

    // Parse command line options
    int opt;
    while ((opt = getopt(argc, argv, "g:j:b:p:i:a:h")) != -1)
    {
        switch (opt)
        {
        case 'g':
            groupIp = optarg;
            break;
        case 'j':
            jsonFile = optarg;
            break;
        case 'b':
            bufferSizeMB = static_cast<size_t>(std::stoi(optarg));
            break;
        case 'p':
            port = std::stoi(optarg);
            break;
        case 'i':
            interface = optarg;
            break;
        case 'a':
            localIp = optarg;
            break;
        case 'h':
            printUsage(argv[0]);
            exit(0);
        default:
            std::cerr << "Unknown option: " << static_cast<char>(opt) << std::endl;
            printUsage(argv[0]);
            exit(1);
        }
    }

    // If JSON file was specified, load config from it
    if (!jsonFile.empty())
    {
        if (!loadConfigFromJson(jsonFile, groupIp, interface, localIp, port, bufferSizeMB))
        {
            exit(1);
        }
    }

    // Validate required parameters
    if (groupIp.empty())
    {
        std::cerr << "Error: Multicast group IP (-g) is required or must be provided in JSON config!" << std::endl;
        printUsage(argv[0]);
        exit(1);
    }

    // Simple validation for IP format
    if (inet_addr(groupIp.c_str()) == INADDR_NONE)
    {
        std::cerr << "Error: Invalid multicast group IP format: " << groupIp << std::endl;
        exit(1);
    }

    // Check if it's in multicast range (224.0.0.0 to 239.255.255.255)
    unsigned int firstOctet = static_cast<unsigned int>(inet_addr(groupIp.c_str()) & 0xFF);
    if (firstOctet < 224 || firstOctet > 239)
    {
        std::cerr << "Warning: IP " << groupIp << " is not in multicast range (224.0.0.0 - 239.255.255.255)" << std::endl;
    }

    // Validate buffer size
    if (bufferSizeMB < 1 || bufferSizeMB > 1024)
    {
        std::cerr << "Warning: Buffer size " << bufferSizeMB
                  << "MB is unusual. Using default (100MB)." << std::endl;
        bufferSizeMB = 100;
    }

    // Create configuration with proper types
    return common::MulticastConfig(
        common::SocketDomain::IPV4,
        common::SocketType::UDP,
        0,
        groupIp,
        port,
        interface,
        localIp,
        8 * common::constants::MEGA_BYTE); // 8MB receive buffer
}

int main(int argc, char *argv[])
{
    try
    {
        std::cout << "Stream Buffer - Multicast Packet Processing\n"
                  << "----------------------------------------" << std::endl;

        // Parse command line and get config
        size_t bufferSizeMB = 0;
        common::MulticastConfig config = ParseCommandLine(argc, argv, bufferSizeMB);

        // Display configuration
        std::cout << "Configuration:\n"
                  << "  Group IP:     " << config.group_ip << "\n"
                  << "  Interface:    " << config.interface_name << "\n"
                  << "  Local IP:     " << config.interface_ip << "\n"
                  << "  Port:         " << config.port << "\n"
                  << "  Buffer Size:  " << bufferSizeMB << "MB\n"
                  << "----------------------------------------" << std::endl;

        // Create and run the buffer processor
        core::BufferProcessor processor(
            config,
            bufferSizeMB * common::constants::MEGA_BYTE);

        processor.Run();

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
