#include "config.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using nlohmann::json;
using testing::HasSubstr;

/// Пустой объект: все поля получают значения по умолчанию.
TEST(ParseConfig, Defaults) {
    auto const cfg = parse_config(json::object());
    EXPECT_EQ(cfg.listen_addr, "0.0.0.0");
    EXPECT_EQ(cfg.port, 8082);
    EXPECT_EQ(cfg.log_level, 6);
    EXPECT_EQ(cfg.data_file, "/var/lib/price-tags/catalog.json");
}

/// Заданные поля переопределяют значения по умолчанию.
TEST(ParseConfig, Values) {
    auto const input = R"({"listen_addr": "127.0.0.1", "port": 9000, "log_level": 7, "data_file": "/tmp/c.json"})";
    SCOPED_TRACE(input);
    auto const cfg = parse_config(json::parse(input));
    EXPECT_EQ(cfg.listen_addr, "127.0.0.1");
    EXPECT_EQ(cfg.port, 9000);
    EXPECT_EQ(cfg.log_level, 7);
    EXPECT_EQ(cfg.data_file, "/tmp/c.json");
}

/// Некорректные значения отклоняются с указанием поля и причины.
TEST(ParseConfig, Errors) {
    struct Case {
        char const *input;
        char const *reason;
    };
    for (auto const &c : {Case{R"([])", "JSON-объектом"}, Case{R"({"port": 0})", "\"port\": 0 вне допустимого диапазона"},
                          Case{R"({"port": "80"})", "\"port\" должно быть целым"}, Case{R"({"port": 1.5})", "\"port\" должно быть целым"},
                          Case{R"({"log_level": 8})", "\"log_level\": 8 вне"}, Case{R"({"listen_addr": 1})", "\"listen_addr\" должно быть непустой"},
                          Case{R"({"data_file": ""})", "\"data_file\" должно быть непустой"}}) {
        SCOPED_TRACE(c.input);
        try {
            (void)parse_config(json::parse(c.input));
            ADD_FAILURE() << "вход " << c.input << ": ожидалось исключение с причиной \"" << c.reason << "\", но конфигурация принята";
        } catch (std::runtime_error const &ex) {
            EXPECT_THAT(ex.what(), HasSubstr(c.reason)) << "вход " << c.input;
        }
    }
}

/// Отсутствующий файл конфигурации: исключение с путём к файлу.
TEST(LoadConfig, MissingFile) {
    try {
        (void)load_config("/nonexistent/cfg.json");
        ADD_FAILURE() << "ожидалось исключение для отсутствующего файла";
    } catch (std::runtime_error const &ex) {
        EXPECT_THAT(ex.what(), HasSubstr("/nonexistent/cfg.json"));
    }
}
