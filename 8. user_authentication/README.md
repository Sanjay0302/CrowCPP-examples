# Secure User Authentication System with Crow C++

A secure web application authentication system built with Crow C++ framework.

## Features

- **Password Security**: SHA-256 hashing with random salt
- **Session Management**: Secure session handling with HTTP-only cookies
- **Password Validation**: Enforces strong passwords (8+ chars, letters + numbers)
- **User Management**: Registration, login, password change, account deactivation
- **Session Cleanup**: Automatic cleanup of expired tokens
- **Thread-Safe**: All operations are thread-safe with mutex protection

## Files

- `auth_manager.h/cpp` - Core authentication logic
- `main.cpp` - Web server with API endpoints
- `login.html` - Login page
- `register.html` - Registration page  
- `dashboard.html` - User dashboard
- `CMakeLists.txt` - Build configuration

## Security Features

1. **Password Hashing**: Uses SHA-256 with 32-byte random salt
2. **Session Tokens**: 32-byte cryptographically secure random tokens
3. **Token Expiry**: 24-hour session timeout
4. **Input Validation**: Password strength requirements
5. **Thread Safety**: Mutex-protected user and token storage

## Building and Running

```bash
mkdir build && cd build
cmake ..
make
cd ..
./build/auth_server
```

## API Endpoints

- `POST /api/login` - User login
- `POST /api/register` - User registration  
- `POST /api/change-password` - Change password
- `GET /api/logout` - User logout

## Web Pages

- `/login` - Login form
- `/register` - Registration form
- `/dashboard` - User dashboard (protected)

Server runs on port 18080.
