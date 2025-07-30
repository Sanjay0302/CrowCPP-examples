#include "crow.h"
#include <sstream>
#include <openssl/ssl.h>

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]()
                         { return "Hello world! (HTTPS Enabled)"; });

    CROW_ROUTE(app, "/hello/<int>")
    ([](int count)
     {
        if (count > 100)
            return crow::response(400);
        std::ostringstream os;
        os << count << " is sent from your browser.";
        return crow::response(os.str()); });

    CROW_ROUTE(app, "/about")([]()
                              { return "This is a Crow C++ web framework example with HTTPS support!"; });

    // configure SSL context to use only TLS 1.2
    asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12);

    ssl_ctx.use_certificate_file("server.crt", asio::ssl::context::pem);
    ssl_ctx.use_private_key_file("server.key", asio::ssl::context::pem);

    // TLS1.2 only configuration
    SSL_CTX_set_min_proto_version(ssl_ctx.native_handle(), TLS1_2_VERSION);
    SSL_CTX_set_max_proto_version(ssl_ctx.native_handle(), TLS1_2_VERSION);

    // Set cipher suites for TLS 1.2
    SSL_CTX_set_cipher_list(ssl_ctx.native_handle(), "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256");

    // Set SSL options
    ssl_ctx.set_options(
        asio::ssl::context::default_workarounds |
        asio::ssl::context::no_sslv2 |
        asio::ssl::context::no_sslv3 |
        asio::ssl::context::no_tlsv1 |
        asio::ssl::context::no_tlsv1_1 |
        asio::ssl::context::no_tlsv1_3);

    // Start the server with HTTPS enabled on port 8443
    app.port(8443)
        .ssl(std::move(ssl_ctx)) // Use our custom SSL context
        .multithreaded()
        .bindaddr("127.0.0.1")
        .port(8443)
        .run();

    return 0;
}
