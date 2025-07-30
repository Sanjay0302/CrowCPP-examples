#include "crow.h"
#include <unordered_set>
#include <mutex>

std::mutex mtx;
std::unordered_set<crow::websocket::connection*> users;

// GPIO states (simulating hardware)
struct GPIOState {
    bool gpio1 = false;
    bool gpio2 = false;
    bool gpio3 = false;
};

GPIOState gpio_state;

void broadcast_gpio_state() {
    std::string message = "{\"type\":\"gpio_update\",\"gpio1\":" 
        + std::string(gpio_state.gpio1 ? "true" : "false")
        + ",\"gpio2\":" + std::string(gpio_state.gpio2 ? "true" : "false")
        + ",\"gpio3\":" + std::string(gpio_state.gpio3 ? "true" : "false") + "}";
    
    std::lock_guard<std::mutex> _(mtx);
    for (auto u : users) {
        u->send_text(message);
    }
}

int main()
{
    crow::SimpleApp app;

    CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection& conn) {
          CROW_LOG_INFO << "new websocket connection from " << conn.get_remote_ip();
          std::lock_guard<std::mutex> _(mtx);
          users.insert(&conn);
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
          CROW_LOG_INFO << "websocket connection closed: " << reason;
          std::lock_guard<std::mutex> _(mtx);
          users.erase(&conn);
      })
      .onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary) {
          std::lock_guard<std::mutex> _(mtx);
          for (auto u : users)
              if (is_binary)
                  u->send_binary(data);
              else
                  u->send_text(data);
      });

    CROW_ROUTE(app, "/")
    ([] {
        char name[256];
        gethostname(name, 256);
        crow::mustache::context x;
        x["servername"] = name;

        auto page = crow::mustache::load("ws.html");
        return page.render(x);
    });

    // HTTP endpoint for GPIO 1 toggle
    CROW_ROUTE(app, "/gpio1").methods("POST"_method)
    ([] {
        gpio_state.gpio1 = !gpio_state.gpio1;
        broadcast_gpio_state();
        return crow::response(200, gpio_state.gpio1 ? "ON" : "OFF");
    });

    // HTTP endpoint for GPIO 2 toggle
    CROW_ROUTE(app, "/gpio2").methods("POST"_method)
    ([] {
        gpio_state.gpio2 = !gpio_state.gpio2;
        broadcast_gpio_state();
        return crow::response(200, gpio_state.gpio2 ? "ON" : "OFF");
    });

    // HTTP endpoint for GPIO 3 toggle
    CROW_ROUTE(app, "/gpio3").methods("POST"_method)
    ([] {
        gpio_state.gpio3 = !gpio_state.gpio3;
        broadcast_gpio_state();
        return crow::response(200, gpio_state.gpio3 ? "ON" : "OFF");
    });

    app.port(8080)
      .multithreaded()
      .run();
}
