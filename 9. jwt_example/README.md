# JWT Middleware Example for Crow C++

This project demonstrates a complete JWT (JSON Web Token) authentication middleware implementation for the Crow C++ web framework. It includes token generation, validation, and route-specific protection with role-based access control.

## Features

- **JWT Middleware**: Route-specific JWT protection extending `crow::ILocalMiddleware`
- **Token Extraction**: Bearer token extraction from Authorization header
- **Token Verification**: HMAC-SHA256 signature verification using OpenSSL
- **Claims Validation**: Expiration, issuer, and audience validation
- **Role-based Access**: Separate middleware for admin-only routes
- **Interactive Demo**: HTML interface for testing JWT functionality

## Architecture

### Core Components

1. **JWTAuthenticator** (`jwt_auth.h/cpp`): Core JWT functionality

   - Token generation with customizable claims
   - Token validation and signature verification
   - Base64 URL encoding/decoding utilities
   - HMAC-SHA256 signature implementation

2. **JWTMiddleware** (`jwt_middleware.h`): Route protection middleware

   - Extends `crow::ILocalMiddleware` for route-specific usage
   - Automatic token extraction and validation
   - Context storage for handler access to user data

3. **AdminJWTMiddleware**: Role-based access control
   - Inherits JWT validation from base middleware
   - Additional role checking for admin access

### JWT Token Structure

```
Header: {"alg": "HS256", "typ": "JWT"}
Payload: {
  "sub": "user_id",
  "username": "username",
  "role": "user/admin",
  "exp": expiration_timestamp,
  "iat": issued_at_timestamp,
  "iss": "jwt_example",
  "aud": "jwt_example_users"
}
```

## Building and Running

### Prerequisites

- CMake 3.16+
- C++17 compatible compiler
- OpenSSL development libraries
- Crow framework (included in `../libs/crow/`)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

### Running the Server

```bash
./jwt_example
```

The server will start on port 18080.

## Default Test Users

- **Admin User**: `admin` / `adminpass123` (admin role)
- **Regular User**: `user` / `userpass123` (user role)

## API Endpoints

### Public Endpoints (No Authentication)

- `GET /` - Main demo page
- `GET /login.html` - Login page
- `GET /api/public` - Public API endpoint

### Authentication

- `POST /api/login` - Login endpoint
  ```json
  {
    "username": "admin",
    "password": "adminpass123"
  }
  ```

### Protected Endpoints (JWT Required)

- `GET /api/protected` - Basic protected content
- `GET /api/profile` - User profile information

### Admin Endpoints (Admin JWT Required)

- `GET /api/admin` - Admin dashboard
- `GET /api/admin/users` - User management

## Usage Example

### 1. Login to get JWT token

```bash
curl -X POST http://localhost:18080/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"adminpass123"}'
```

### 2. Use token to access protected routes

```bash
curl -H "Authorization: Bearer YOUR_JWT_TOKEN" \
  http://localhost:18080/api/protected
```

## Middleware Usage in Routes

```cpp
#include "jwt_middleware.h"

// Create app with middlewares
crow::App<JWTMiddleware, AdminJWTMiddleware> app;

// Protected route
CROW_ROUTE(app, "/protected")
.CROW_MIDDLEWARES(app, JWTMiddleware)
([&](const crow::request& req) {
    auto& ctx = app.get_context<JWTMiddleware>(req);
    // Access user data from ctx.payload
    return "Hello " + ctx.payload.username;
});

// Admin-only route
CROW_ROUTE(app, "/admin")
.CROW_MIDDLEWARES(app, AdminJWTMiddleware)
([&](const crow::request& req) {
    auto& ctx = app.get_context<AdminJWTMiddleware>(req);
    // Only admins can access this
    return "Admin panel for " + ctx.payload.username;
});
```

## Security Features

- **HMAC-SHA256 Signature**: Cryptographically secure token signing
- **Token Expiration**: Configurable token lifetime (default: 24 hours)
- **Bearer Token Extraction**: Standard HTTP Authorization header
- **Role-based Access**: Separate middleware for different permission levels
- **Input Validation**: Comprehensive token format and claim validation

## Demo Interface

The included HTML interface provides:

- Interactive login with test users
- Real-time JWT token display
- API endpoint testing
- Response logging and visualization
- Role-based access demonstration

## Key Implementation Details

### Token Generation

```cpp
std::string token = jwt_auth->generate_token(
    user_id,
    username,
    role,
    24  // expires in 24 hours
);
```

### Middleware Context Access

```cpp
auto& ctx = app.get_context<JWTMiddleware>(req);
std::string user_id = ctx.payload.user_id;
std::string role = ctx.payload.role;
```

### Error Handling

- Invalid token format: HTTP 401
- Expired tokens: HTTP 401
- Missing Authorization header: HTTP 401
- Insufficient permissions: HTTP 403

## Customization

### Custom Secret Key

```cpp
auto jwt_auth = std::make_shared<JWTAuthenticator>(
    "your-secret-key",
    "your-issuer",
    "your-audience"
);
```

### Custom Token Expiration

```cpp
std::string token = jwt_auth->generate_token(
    user_id, username, role,
    72  // expires in 72 hours
);
```
