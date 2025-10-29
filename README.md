# e-space

## Description

e-space is a C++ server-client application using gRPC for communication and Protocol Buffers for data serialization. The project demonstrates a real-time auction system, where clients can register, add products, view listings, and place bids, all coordinated through a gRPC server.

**Warning: This approach is extremely verbose and painful for real applications.**

**If you want to suffer, this is the way to go with servers. Otherwise, use REST lol.**

## Table of Contents

- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Windows/Linux/macOS](#windowslinuxmacos)
- [Usage](#usage)
- [License](#license)
- [Contact](#contact)

## Installation

### Prerequisites

- C++17 or newer compiler
- CMake >= 3.15
- gRPC and Protocol Buffers libraries (with C++ plugins)
- SDL3 (for client GUI)
- Vulkan SDK (for client GUI)

### Windows/Linux/macOS

1. Clone the repo: `git clone https://github.com/HugeErick/e-space.git`
2. Build the server:

   ```bash
   cd server
   mkdir -p build && cd build
   cmake ..
   make
   ```

3. Build the client:

   ```bash
   cd ../../client
   make
   ```

## Usage

1. Start the server:

   ```bash
   cd server/build
   ./server
   ```

2. Run the client:

   ```bash
   cd ../client
   ./client
   ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

Erick Gonzalez Parada - <erick.parada101@gmail.com>

Project Link: [https://github.com/HugeErick/e-space](https://github.com/HugeErick/e-space)
