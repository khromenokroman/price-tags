#include "web_server.hpp"

#include <fmt/format.h>
#include <syslog.h>

#include <stdexcept>

#include "index_page.hpp"

WebServer::WebServer(Config const &config) : m_config{config} {}

void WebServer::run() {
    m_server.Get("/", [](httplib::Request const &req, httplib::Response &res) {
        syslog(LOG_DEBUG, "Поступил запрос('/') от %s:%d", req.remote_addr.c_str(), req.remote_port);
        res.set_content(index_page().data(), index_page().size(), "text/html; charset=utf-8");
    });

    m_server.set_socket_options([](socket_t sock) {
        int const opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    });
    if (!m_server.bind_to_port(m_config.listen_addr, m_config.port)) {
        auto const msg = fmt::format("Не удалось занять адрес {}:{}", m_config.listen_addr, m_config.port);
        syslog(LOG_ERR, "%s", msg.c_str());
        throw std::runtime_error(msg);
    }
    syslog(LOG_NOTICE, "Запущен сервер на http://%s:%d", m_config.listen_addr.c_str(), m_config.port);
    m_server.listen_after_bind();
    syslog(LOG_NOTICE, "Сервер остановлен");
}

void WebServer::stop() { m_server.stop(); }
