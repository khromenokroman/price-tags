#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

/**
 * @brief Конфигурация приложения.
 */
struct Config {
    std::string listen_addr{"0.0.0.0"};                        ///< Адрес, на котором слушает HTTP-сервер.
    std::string data_file{"/var/lib/price-tags/catalog.json"}; ///< Файл справочников организаций и товаров.
    int port{8082};                                            ///< Порт HTTP-сервера.
    int log_level{6};                                          ///< Уровень логирования syslog.
};

/**
 * @brief Разбирает и проверяет конфигурацию из JSON. Все поля необязательны.
 * @param j JSON-объект конфигурации.
 * @return Проверенная конфигурация.
 * @throw std::runtime_error при некорректной конфигурации.
 */
[[nodiscard]] Config parse_config(nlohmann::json const &j);

/**
 * @brief Загружает конфигурацию из файла.
 * @param path Путь к JSON-файлу конфигурации.
 * @return Проверенная конфигурация.
 * @throw std::runtime_error если файл не открывается или содержит ошибки.
 */
[[nodiscard]] Config load_config(std::string_view path);
