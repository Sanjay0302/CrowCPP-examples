# CrowCPP-examples
This Repo has the CrowCPP real world scenario based examples with security implementation using openssl

Crow (C++ Microframework) : https://github.com/CrowCpp/Crow/blob/master/LICENSE

Asio (Networking and Thread Lib dependency) standalone : https://think-async.com/Asio/

Optional dependency : 
- (Compression) zlib : https://github.com/madler/zlib/blob/develop/LICENSE
- (SSL) openssl: https://github.com/openssl/openssl/blob/master/LICENSE.txt
- (DB) sqlite : https://sqlite.org/copyright.html
- (DB) valkeydb : https://github.com/valkey-io/valkey/blob/unstable/COPYING

---

# Installation


## Asio Standalone tar.gz from github tag asio-1.30.2

https://think-async.com/Asio/ 

Latest stable release as on `20/07/2025 11:22 PM` is `1.30.2`

Asio download from tag asio-1.30.2: https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz

This mentioned link is routed to https://codeload.github.com/chriskohlhoff/asio/tar.gz/refs/tags/asio-1-30-2 ; so use any think the links will be auto redirected.

preferably use the github url : the codeload url downloads the same file but forgets to add the tar.gz extension to the saved file, which we have to manually rename the file with tar.gz and extract using `tar xvf filename.tar.gz`

- Release Doc (non-boost) : https://think-async.com/Asio/asio-1.30.2/doc/
- Release Doc (boost) : https://think-async.com/Asio/boost_asio_1_30_2/doc/html/boost_asio.html

Asio 1.30.2 is also included in Boost 1.85

----

### Crow CPP from releases

The Latest from github as on 20/07/2025 11:25 PM was v1.2.1.2 : https://github.com/CrowCpp/Crow/releases/tag/v1.2.1.2

A single header only library `crow_all.h` for this version can be downloaded from here (you can find that on the above release page also): https://github.com/CrowCpp/Crow/releases/download/v1.2.1.2/crow_all.h

Or find the tar.gz release here (you can find that on the above release page also): 
https://github.com/CrowCpp/Crow/releases/download/v1.2.1.2/Crow-1.2.1-Linux.tar.gz

Other installation method like deb can be found on the mentioned release page

---

Extract, Move and rename the folders to match the below structure

```bash
 $ tree -L 2
.
├── file_upload_server_example
│   ├── aarch64-toolchain.cmake
│   ├── build-aarch64
│   ├── CMakeLists.txt
│   └── main.cpp
└── libs
    ├── asio
    └── crow

6 directories, 3 files
```



