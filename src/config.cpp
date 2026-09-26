#include "config.hpp"

#include <fmt/format.h>

#include <cerrno>
#include <fstream>
#include <stdexcept>
#include <system_error>

namespace {

void read_int(nlohmann::json const &j, std::string_view key, int &out, int min, int max) {
    if (!j.contains(key)) {
        return;
    }
    auto const &v = j.at(key);
    if (!v.is_number_integer()) {
        throw std::runtime_error(fmt::format("Поле \"{}\" должно быть целым числом", key));
    }
    auto const value = v.get<std::int64_t>();
    if (value < min || value > max) {
        throw std::runtime_error(fmt::format("Поле \"{}\": {} вне допустимого диапазона {}-{}", key, value, min, max));
    }
    out = static_cast<int>(value);
}

void read_string(nlohmann::json const &j, std::string_view key, std::string &out) {
    if (!j.contains(key)) {
        return;
    }
    auto const &v = j.at(key);
    if (!v.is_string() || v.get<std::string>().empty()) {
        throw std::runtime_error(fmt::format("Поле \"{}\" должно быть непустой строкой", key));
    }
    out = v.get<std::string>();
}

} // namespace

Config parse_config(nlohmann::json const &j) {
    if (!j.is_object()) {
        throw std::runtime_error("Конфигурация должна быть JSON-объектом");
    }

    Config cfg;
    read_string(j, "listen_addr", cfg.listen_addr);
    read_string(j, "data_file", cfg.data_file);
    read_int(j, "port", cfg.port, 1, 65535);
    read_int(j, "log_level", cfg.log_level, 0, 7);
    return cfg;
}

Config load_config(std::string_view path) {
    std::ifstream file{std::string{path}};
    if (!file.is_open()) {
        auto const err = errno;
        throw std::runtime_error(fmt::format("Не могу открыть настройки({}): {}", path, std::generic_category().message(err)));
    }
    nlohmann::json j;
    try {
        file >> j;
    } catch (nlohmann::json::parse_error const &ex) {
        throw std::runtime_error(fmt::format("Ошибка разбора настроек({}): {}", path, ex.what()));
    }
    return parse_config(j);
}
