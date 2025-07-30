# HTML Server with HTTPS and HTTP to HTTPS Redirection

A secure web server built with the Crow C++ framework that serves HTML files over HTTPS and automatically redirects all HTTP traffic to HTTPS.

## Features

- **HTTPS Support**: Full SSL/TLS encryption using OpenSSL
- **HTTP to HTTPS Redirection**: Automatic redirection from HTTP (port 8080) to HTTPS (port 8443)
- **Static File Serving**: Serves HTML, CSS, JavaScript, and other static files from the `public/` directory
- **RESTful API**: JSON API endpoints for server status and other operations
- **Security**: Path traversal protection and proper MIME type handling
- **Multi-threaded**: Concurrent request handling for better performance
- **Modern Web Standards**: Responsive design with modern CSS and JavaScript

## Project Structure

```txt
html-serving/
├── CMakeLists.txt          # CMake build configuration
├── main.cpp                # Main server application
├── README.md              # This file
├── ssl_certs/             # SSL certificates directory
│   ├── server.crt         # SSL certificate
│   └── server.key         # SSL private key
└── public/                # Static web files
    ├── index.html         # Main homepage
    ├── about.html         # About page
    ├── style.css          # CSS stylesheet
    └── app.js             # Client-side JavaScript
```

## Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- OpenSSL development libraries
- ZLIB development libraries
- Crow C++ framework (included in ../libs/)
- ASIO library (included in ../libs/)

### Ubuntu/Debian Installation

```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev zlib1g-dev
```

### CentOS/RHEL Installation

```bash
sudo yum install gcc-c++ cmake openssl-devel zlib-devel
```

## Building and Running

1. **Clone or navigate to the project directory:**

   ```bash
   cd html-serving
   ```

2. **Create a build directory:**

   ```bash
   mkdir build
   cd build
   ```

3. **Configure and build the project:**

   ```bash
   cmake ..
   make
   ```

4. **Run the server:**
   ```bash
   ./HTMLServer
   ```

## Usage

Once the server is running, you can access it at:

- **HTTP (redirects to HTTPS)**: http://localhost:8080
- **HTTPS (secure)**: https://localhost:8443

### Available Endpoints

- `/` - Homepage with server information
- `/about.html` - About page with technical details
- `/api/status` - JSON API endpoint returning server status
- `/style.css` - CSS stylesheet
- `/app.js` - Client-side JavaScript

### Testing HTTP to HTTPS Redirection

1. Open your browser and go to: http://localhost:8080
2. The server will automatically redirect you to: https://localhost:8443
3. You should see the secure webpage with the 🔒 lock icon in your browser

## SSL Certificates

The project uses self-signed SSL certificates for development purposes. Your browser will show a security warning which you can safely ignore for local development.

### Generating New SSL Certificates (Optional)

If you need to generate new SSL certificates:

```bash
# Generate private key
openssl genrsa -out ssl_certs/server.key 2048

# Generate certificate signing request
openssl req -new -key ssl_certs/server.key -out ssl_certs/server.csr

# Generate self-signed certificate
openssl x509 -req -days 365 -in ssl_certs/server.csr -signkey ssl_certs/server.key -out ssl_certs/server.crt

# Clean up
rm ssl_certs/server.csr
```

## Configuration

You can modify the following settings in `main.cpp`:

- **HTTP Port**: Change `8080` to your desired HTTP port
- **HTTPS Port**: Change `8443` to your desired HTTPS port
- **SSL Certificate Paths**: Update paths to your SSL certificate and key files
- **Static Files Directory**: Modify `public/` directory path as needed

## Security Features

1. **Path Traversal Protection**: Prevents access to files outside the public directory
2. **HTTPS Enforcement**: All sensitive operations require HTTPS
3. **Proper MIME Types**: Automatic MIME type detection for different file types
4. **404 Error Handling**: Graceful handling of missing files
5. **Header Security**: Proper HTTP headers for security

## Development

### Adding New Routes

To add new routes, edit `main.cpp` and add new `CROW_ROUTE` definitions:

```cpp
// Example: Add a new API endpoint
CROW_ROUTE(https_app, "/api/hello")
([](){
    crow::json::wvalue response;
    response["message"] = "Hello, World!";
    return crow::response(200, response.dump());
});
```

### Logs

The server outputs logs to the console

- "Starting HTTP server on port 8080 (redirects to HTTPS)..."
- "Starting HTTPS server on port 8443..."
