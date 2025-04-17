# Stream Buffer

A high-performance, thread-safe buffer implementation for multicast network packet processing.

## Features

- Thread-safe buffer management using POSIX threads
- Multicast UDP socket implementation
- Efficient memory management with buffer compaction
- Extensible message processing architecture
- Real-time data processing capabilities

## Requirements

- C++11 or later
- POSIX-compliant system (Linux, macOS)
- GCC 11 recommended (via devtoolset-11 on CentOS 7)

## Building

### Using Makefile (Recommended for CentOS 7)

```bash
cd stream_buffer
make
```

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
```

### CentOS 7 Docker Environment

For testing compatibility with CentOS 7 and GCC 11, you can use the provided Docker environment:

```bash
# Build the Docker image
docker build -t stream_buffer:centos7 .

# Run the container with an interactive shell
docker run -it stream_buffer:centos7

# Inside the container, build and test the project
cd /app/stream_buffer
make clean
make cpp11-check  # Verify C++11 compatibility
make
make test
```

## Usage

### Basic usage

```bash
# Using direct multicast group IP
./stream_buffer -g 225.0.0.1 -p 10000 -i eth0 -a 192.168.1.10

# Using a JSON configuration file
./stream_buffer -j config.json
```

### Configuration options

| Option             | Description                              |
| ------------------ | ---------------------------------------- |
| `-g <group_ip>`    | Multicast group IP address               |
| `-j <json_file>`   | JSON configuration file                  |
| `-b <buffer_size>` | Buffer size in MB (default: 100)         |
| `-p <port>`        | Port number (default: 10000)             |
| `-i <interface>`   | Network interface (default: en049.135)   |
| `-a <address>`     | Local IP address (default: 10.71.205.68) |
| `-h`               | Show help message                        |

### JSON Configuration

Example `config.json`:

```json
{
  "group_ip": "225.0.0.1",
  "interface": "eth0",
  "local_ip": "192.168.1.10",
  "port": 10000,
  "buffer_size_mb": 200
}
```

## Architecture

The Stream Buffer project consists of several key components:

1. **Buffer Management**: Efficient memory management with dynamic compaction
2. **Thread Synchronization**: Thread-safe operations with mutexes and condition variables
3. **Multicast Networking**: UDP socket wrapper for multicast data reception
4. **Message Processing**: Extensible framework for processing received data

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details. 

## CI/CD
![C++ CI with CentOS 7](https://github.com/iinoshirozheng/stream_buffer/actions/workflows/main.yml/badge.svg)