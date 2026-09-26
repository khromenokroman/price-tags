#include "api.hpp"

#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdlib.h>

#include <filesystem>
#include <thread>

using nlohmann::json;
using testing::HasSubstr;

namespace {

/**
 * @brief Поднимает HTTP-сервер с API на свободном порту 127.0.0.1 и временным файлом справочников.
 */
class ApiTest : public testing::Test {
   protected:
    void SetUp() override {
        std::string tmpl = (std::filesystem::temp_directory_path() / "price-tags-api-XXXXXX").string();
        ASSERT_NE(mkdtemp(tmpl.data()), nullptr) << "не удалось создать временный каталог " << tmpl;
        m_dir = tmpl;
        m_catalog = std::make_unique<Catalog>((m_dir / "catalog.json").string());
        register_api(m_server, *m_catalog);
        m_port = m_server.bind_to_any_port("127.0.0.1");
        ASSERT_GT(m_port, 0) << "не удалось занять порт";
        m_thread = std::thread{[this] { m_server.listen_after_bind(); }};
        m_server.wait_until_ready();
        m_client = std::make_unique<httplib::Client>("127.0.0.1", m_port);
    }

    void TearDown() override {
        m_server.stop();
        if (m_thread.joinable()) {
            m_thread.join();
        }
        std::filesystem::remove_all(m_dir);
    }

    /// Проверяет код ответа и возвращает разобранное тело (null, если тела нет).
    json expect(httplib::Result const &r, int status, std::string_view what) {
        EXPECT_TRUE(r) << what << ": нет ответа, ошибка " << httplib::to_string(r.error());
        if (!r) {
            return {};
        }
        EXPECT_EQ(r->status, status) << what << ": тело ответа " << r->body;
        return r->body.empty() ? json{} : json::parse(r->body);
    }

    httplib::Result post(std::string const &path, std::string const &body) { return m_client->Post(path, body, "application/json"); }
    httplib::Result put(std::string const &path, std::string const &body) { return m_client->Put(path, body, "application/json"); }

    httplib::Server m_server;
    std::filesystem::path m_dir;
    std::unique_ptr<Catalog> m_catalog;
    std::unique_ptr<httplib::Client> m_client;
    std::thread m_thread;
    int m_port{0};
};

} // namespace

/// Полный цикл по товарам: добавление, список, изменение, удаление; изменения доходят до Catalog.
TEST_F(ApiTest, ProductsCrud) {
    auto const added = expect(post("/api/products", R"({"name": " Хлеб ", "unit": "шт", "price_kop": 4590})"), 201, "POST товара");
    ASSERT_TRUE(added.is_object());
    EXPECT_EQ(added.at("name"), "Хлеб");
    EXPECT_EQ(added.at("article"), "");
    auto const id = added.at("id").get<std::uint64_t>();

    auto const list = expect(m_client->Get("/api/products"), 200, "GET списка");
    ASSERT_EQ(list.size(), 1U) << list.dump();
    EXPECT_EQ(list[0], added);

    auto const path = fmt::format("/api/products/{}", id);
    auto const updated = expect(put(path, R"({"id": 999, "name": "Хлеб", "unit": "шт", "price_kop": 5000})"), 200, "PUT товара");
    EXPECT_EQ(updated.at("id"), id) << "id берётся из пути, а не из тела";
    EXPECT_EQ(m_catalog->products().at(0).price_kop, 5000);

    expect(m_client->Delete(path), 204, "DELETE товара");
    EXPECT_TRUE(m_catalog->products().empty());
}

/// Полный цикл по организациям.
TEST_F(ApiTest, OrganizationsCrud) {
    auto const added = expect(post("/api/organizations", R"({"name": "ИП Иванов"})"), 201, "POST организации");
    auto const path = fmt::format("/api/organizations/{}", added.at("id").get<std::uint64_t>());
    EXPECT_EQ(expect(put(path, R"({"name": "ООО Ромашка"})"), 200, "PUT организации").at("name"), "ООО Ромашка");
    EXPECT_EQ(expect(m_client->Get("/api/organizations"), 200, "GET списка").size(), 1U);
    expect(m_client->Delete(path), 204, "DELETE организации");
    EXPECT_TRUE(m_catalog->organizations().empty());
}

/// Ошибки: код ответа и текст причины в поле error.
TEST_F(ApiTest, Errors) {
    struct Case {
        char const *method;
        char const *path;
        char const *body;
        int status;
        char const *reason;
    };
    for (auto const &c : {Case{"POST", "/api/products", "{", 400, "Некорректный JSON"},
                          Case{"POST", "/api/products", R"({"name": "", "unit": "шт", "price_kop": 1})", 400, "не может быть пустым"},
                          Case{"POST", "/api/products", R"({"name": "Х", "unit": "шт", "price_kop": 1.5})", 400, "целым числом копеек"},
                          Case{"POST", "/api/organizations", R"([])", 400, "JSON-объектом"},
                          Case{"PUT", "/api/products/42", R"({"name": "Х", "unit": "шт", "price_kop": 1})", 404, "Товар не найден (id 42)"},
                          Case{"PUT", "/api/products/99999999999999999999999", R"({})", 404, "Некорректный id"},
                          Case{"DELETE", "/api/organizations/42", "", 404, "Организация не найдена (id 42)"},
                          Case{"DELETE", "/api/products/0", "", 404, "Некорректный id"}}) {
        auto const what = fmt::format("{} {} {}", c.method, c.path, c.body);
        SCOPED_TRACE(what);
        auto const r = std::string_view{c.method} == "POST"  ? post(c.path, c.body)
                       : std::string_view{c.method} == "PUT" ? put(c.path, c.body)
                                                             : m_client->Delete(c.path);
        auto const body = expect(r, c.status, what);
        ASSERT_TRUE(body.is_object() && body.contains("error")) << "ожидалось {\"error\": ...}, получено " << body.dump();
        EXPECT_THAT(body.at("error").get<std::string>(), HasSubstr(c.reason));
    }
    EXPECT_TRUE(m_catalog->products().empty());
    EXPECT_TRUE(m_catalog->organizations().empty());
}

/// Ошибка сохранения на диск: 500 и текст причины, справочник не меняется.
TEST_F(ApiTest, SaveFailure) {
    std::filesystem::remove_all(m_dir);
    auto const body = expect(post("/api/organizations", R"({"name": "ИП Иванов"})"), 500, "POST при удалённом каталоге");
    ASSERT_TRUE(body.contains("error")) << body.dump();
    EXPECT_THAT(body.at("error").get<std::string>(), HasSubstr("Не удалось создать файл"));
    EXPECT_TRUE(m_catalog->organizations().empty());
}
