#include "crow.h"
#include <fstream>
#include <filesystem>

int main()
{
    crow::SimpleApp app;

    // Route to list available files
    CROW_ROUTE(app, "/files")
      .methods(crow::HTTPMethod::Get)([](const crow::request& req) {
          std::string html = "<html><body><h2>Available Files for Download</h2><ul>";
          
          try {
              for (const auto& entry : std::filesystem::directory_iterator(".")) {
                  if (entry.is_regular_file()) {
                      std::string filename = entry.path().filename().string();
                      html += "<li><a href=\"/download/" + filename + "\">" + filename + "</a></li>";
                  }
              }
          } catch (const std::exception& e) {
              CROW_LOG_ERROR << "Error listing files: " << e.what();
              return crow::response(500, "Error listing files");
          }
          
          html += "</ul></body></html>";
          return crow::response(200, html);
      });

    // Route to download a specific file
    CROW_ROUTE(app, "/download/<string>")
      .methods(crow::HTTPMethod::Get)([](const crow::request& req, const std::string& filename) {
          CROW_LOG_INFO << "Download request for file: " << filename;
          
          // Security check: prevent directory traversal attacks
          if (filename.find("..") != std::string::npos || filename.find("/") != std::string::npos) {
              CROW_LOG_WARNING << "Invalid filename detected: " << filename;
              return crow::response(400, "Invalid filename");
          }
          
          // Check if file exists
          if (!std::filesystem::exists(filename)) {
              CROW_LOG_WARNING << "File not found: " << filename;
              return crow::response(404, "File not found");
          }
          
          // Read file content
          std::ifstream file(filename, std::ios::binary);
          if (!file) {
              CROW_LOG_ERROR << "Failed to open file: " << filename;
              return crow::response(500, "Failed to read file");
          }
          
          // Get file size
          file.seekg(0, std::ios::end);
          size_t file_size = file.tellg();
          file.seekg(0, std::ios::beg);
          
          // Read file content into string
          std::string file_content(file_size, '\0');
          file.read(&file_content[0], file_size);
          file.close();
          
          CROW_LOG_INFO << "Successfully read file: " << filename << " (size: " << file_size << " bytes)";
          
          // Create response with appropriate headers
          crow::response response(200, file_content);
          response.add_header("Content-Type", "application/octet-stream");
          response.add_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
          response.add_header("Content-Length", std::to_string(file_size));
          
          return response;
      });

    // Route to get file info (metadata)
    CROW_ROUTE(app, "/info/<string>")
      .methods(crow::HTTPMethod::Get)([](const crow::request& req, const std::string& filename) {
          CROW_LOG_INFO << "Info request for file: " << filename;
          
          // Security check: prevent directory traversal attacks
          if (filename.find("..") != std::string::npos || filename.find("/") != std::string::npos) {
              CROW_LOG_WARNING << "Invalid filename detected: " << filename;
              return crow::response(400, "Invalid filename");
          }
          
          // Check if file exists
          if (!std::filesystem::exists(filename)) {
              CROW_LOG_WARNING << "File not found: " << filename;
              return crow::response(404, "File not found");
          }
          
          try {
              auto file_size = std::filesystem::file_size(filename);
              auto last_write_time = std::filesystem::last_write_time(filename);
              
              // Convert file time to string (simplified)
              auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                  last_write_time - std::filesystem::file_time_type::clock::now() + 
                  std::chrono::system_clock::now());
              std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
              
              crow::json::wvalue response_json;
              response_json["filename"] = filename;
              response_json["size"] = static_cast<int64_t>(file_size);
              response_json["last_modified"] = std::ctime(&cftime);
              
              return crow::response(200, response_json);
              
          } catch (const std::exception& e) {
              CROW_LOG_ERROR << "Error getting file info: " << e.what();
              return crow::response(500, "Error getting file information");
          }
      });

    // Root route with basic information
    CROW_ROUTE(app, "/")
    ([](const crow::request& req) {
        std::string html = R"(
            <html>
            <body>
                <h1>File Download Server</h1>
                <p>Available endpoints:</p>
                <ul>
                    <li><a href="/files">/files</a> - List all available files</li>
                    <li>/download/&lt;filename&gt; - Download a specific file</li>
                    <li>/info/&lt;filename&gt; - Get file information</li>
                </ul>
            </body>
            </html>
        )";
        return crow::response(200, html);
    });

    // Enable all logs
    app.loglevel(crow::LogLevel::Debug);

    CROW_LOG_INFO << "File Download Server starting on port 18081";
    app.port(18081)
      .multithreaded()
      .run();

    return 0;
}
