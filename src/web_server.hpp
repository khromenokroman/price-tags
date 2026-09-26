#pragma once
#include <httplib.h>

#include "catalog.hpp"
#include "config.hpp"

/**
 * @brief HTTP-сервер: страница печати ценников и JSON API.
 */
class WebServer {
   public:
    /**
     * @brief Конструктор.
     * @param config Конфигурация (адрес и порт); должна жить дольше сервера.
     * @param catalog Справочники; должны жить дольше сервера.
     */
    WebServer(Config const &config, Catalog &catalog);

    /**
     * @brief Регистрирует маршруты и запускает сервер. Блокирует поток до вызова stop().
     * @throw std::runtime_error если не удалось занять адрес и порт.
     */
    void run();

    /**
     * @brief Останавливает сервер. Может вызываться из другого потока.
     */
    void stop();

   private:
    httplib::Server m_server; // 824
    Config const &m_config;   // 8
    Catalog &m_catalog;       // 8
};
