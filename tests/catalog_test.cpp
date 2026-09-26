#include "catalog.hpp"

#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdlib.h>

#include <filesystem>
#include <fstream>

using nlohmann::json;
using testing::HasSubstr;

namespace {

/**
 * @brief Тест с отдельным временным каталогом для файла справочников.
 */
class CatalogTest : public testing::Test {
   protected:
    void SetUp() override {
        std::string tmpl = (std::filesystem::temp_directory_path() / "price-tags-test-XXXXXX").string();
        ASSERT_NE(mkdtemp(tmpl.data()), nullptr) << "не удалось создать временный каталог " << tmpl;
        m_dir = tmpl;
        m_file = (m_dir / "catalog.json").string();
    }

    void TearDown() override { std::filesystem::remove_all(m_dir); }

    void write(std::string_view text) const { std::ofstream{m_file} << text; }

    [[nodiscard]] std::string read() const {
        std::ifstream f{m_file};
        return {std::istreambuf_iterator<char>{f}, {}};
    }

    std::filesystem::path m_dir;
    std::string m_file;
};

Product product(std::string name, std::int64_t price_kop, std::string unit = "шт") {
    Product p;
    p.name = std::move(name);
    p.unit = std::move(unit);
    p.price_kop = price_kop;
    return p;
}

Organization organization(std::string name) {
    Organization o;
    o.name = std::move(name);
    return o;
}

} // namespace

/// Файла ещё нет: справочники пусты, файл не создаётся до первого изменения.
TEST_F(CatalogTest, MissingFileIsEmpty) {
    Catalog const c{m_file};
    EXPECT_TRUE(c.organizations().empty());
    EXPECT_TRUE(c.products().empty());
    EXPECT_FALSE(std::filesystem::exists(m_file)) << "файл " << m_file << " создан без изменений";
}

/// Добавление назначает id, обрезает пробелы и сразу сохраняет на диск.
TEST_F(CatalogTest, AddAndReload) {
    {
        Catalog c{m_file};
        auto const org = c.add_organization(organization("  ИП Иванов  "));
        EXPECT_EQ(org.id, 1U);
        EXPECT_EQ(org.name, "ИП Иванов") << "пробелы по краям должны обрезаться";
        auto p = product("Хлеб", 4590);
        p.article = "A-1";
        auto const added = c.add_product(p);
        EXPECT_EQ(added.id, 2U) << "id общий для организаций и товаров";
    }
    Catalog const c{m_file};
    ASSERT_EQ(c.organizations().size(), 1U) << "файл:\n" << read();
    EXPECT_EQ(c.organizations()[0].name, "ИП Иванов");
    ASSERT_EQ(c.products().size(), 1U) << "файл:\n" << read();
    auto const p = c.products()[0];
    EXPECT_EQ(p.id, 2U);
    EXPECT_EQ(p.name, "Хлеб");
    EXPECT_EQ(p.unit, "шт");
    EXPECT_EQ(p.article, "A-1");
    EXPECT_EQ(p.price_kop, 4590);
    EXPECT_FALSE(std::filesystem::exists(m_file + ".tmp")) << "временный файл не удалён после сохранения";
}

/// Изменение и удаление по id; id удалённых записей не переиспользуются.
TEST_F(CatalogTest, UpdateRemoveAndIdsNotReused) {
    Catalog c{m_file};
    auto p = c.add_product(product("Молоко", 8900, "л"));
    p.price_kop = 9500;
    EXPECT_EQ(c.update_product(p).price_kop, 9500);
    c.remove_product(p.id);
    EXPECT_TRUE(c.products().empty());
    auto const next = c.add_product(product("Кефир", 7000));
    EXPECT_GT(next.id, p.id) << "id удалённого товара " << p.id << " выдан повторно";

    Catalog const reloaded{m_file};
    ASSERT_EQ(reloaded.products().size(), 1U);
    EXPECT_EQ(reloaded.products()[0].name, "Кефир");
}

/// Изменение и удаление несуществующей записи: NotFoundError, файл не меняется.
TEST_F(CatalogTest, NotFound) {
    Catalog c{m_file};
    auto const org = c.add_organization(organization("ООО Ромашка"));
    auto const before = read();
    auto missing = org;
    missing.id = 100;
    EXPECT_THROW(c.update_organization(missing), NotFoundError);
    EXPECT_THROW(c.remove_organization(100), NotFoundError);
    EXPECT_THROW(c.remove_product(org.id), NotFoundError) << "id организации не должен находиться среди товаров";
    try {
        c.remove_product(100);
    } catch (NotFoundError const &ex) {
        EXPECT_THAT(ex.what(), HasSubstr("id 100"));
    }
    EXPECT_EQ(read(), before);
}

/// Некорректные данные отклоняются с причиной, справочник не меняется.
TEST_F(CatalogTest, Validation) {
    Catalog c{m_file};
    struct Case {
        Product product;
        char const *reason;
    };
    for (auto const &[p, reason] : {Case{product("   ", 100), "Наименование товара: значение не может быть пустым"},
                                    Case{product("Хлеб", 100, ""), "Единица измерения: значение не может быть пустым"},
                                    Case{product("Хлеб", -1), "Цена -1 коп. вне допустимого диапазона"},
                                    Case{product("Хлеб", Catalog::MAX_PRICE_KOP + 1), "вне допустимого диапазона"},
                                    Case{product(std::string(Catalog::MAX_TEXT_LEN + 1, 'x'), 1), "превышает 256"}}) {
        SCOPED_TRACE(fmt::format("name=\"{}\" unit=\"{}\" price_kop={}", p.name.substr(0, 20), p.unit, p.price_kop));
        try {
            (void)c.add_product(p);
            ADD_FAILURE() << "ожидалась ValidationError с причиной \"" << reason << "\", товар принят";
        } catch (ValidationError const &ex) {
            EXPECT_THAT(ex.what(), HasSubstr(reason));
        }
    }
    EXPECT_THROW(c.add_organization(organization("")), ValidationError);
    EXPECT_TRUE(c.products().empty());
    EXPECT_FALSE(std::filesystem::exists(m_file)) << "файл создан, хотя ни одно изменение не прошло проверку";
}

/// Ошибка записи: исключение, состояние в памяти не меняется.
TEST_F(CatalogTest, SaveFailureKeepsState) {
    Catalog c{(m_dir / "nonexistent" / "catalog.json").string()};
    EXPECT_THROW(c.add_product(product("Хлеб", 100)), std::runtime_error);
    EXPECT_TRUE(c.products().empty()) << "товар остался в памяти, хотя не сохранён";
}

/// Испорченный файл: исключение с путём к файлу, файл не перезаписывается.
TEST_F(CatalogTest, CorruptedFile) {
    for (
        auto const *text :
        {R"({"version": 1, "next_id": 1, "organizations": [], "products": [)", R"({"version": 2, "next_id": 1, "organizations": [], "products": []})",
         R"({"version": 1, "organizations": [], "products": []})",
         R"({"version": 1, "next_id": 5, "organizations": [{"id": 1, "name": "А"}], "products": [{"id": 1, "name": "Б", "unit": "шт", "price_kop": 1}]})",
         R"({"version": 1, "next_id": 2, "organizations": [{"id": 3, "name": "А"}], "products": []})",
         R"({"version": 1, "next_id": 5, "organizations": [], "products": [{"id": 1, "name": "Б", "unit": "шт", "price_kop": -5}]})"}) {
        SCOPED_TRACE(text);
        write(text);
        try {
            Catalog const c{m_file};
            ADD_FAILURE() << "испорченный файл загружен без ошибки";
        } catch (std::runtime_error const &ex) {
            EXPECT_THAT(ex.what(), HasSubstr(m_file));
        }
        EXPECT_EQ(read(), text) << "испорченный файл изменён";
    }
}

/// Чтение из JSON: проверка типов полей.
TEST(ProductFromJson, Types) {
    auto const p = product_from_json(json::parse(R"({"id": 7, "name": "Сыр", "unit": "кг", "price_kop": 99900})"));
    EXPECT_EQ(p.id, 7U);
    EXPECT_EQ(p.article, "") << "артикул необязателен";
    EXPECT_EQ(p.price_kop, 99900);
    struct Case {
        char const *input;
        char const *reason;
    };
    for (auto const &c :
         {Case{R"([])", "JSON-объектом"}, Case{R"({"unit": "шт", "price_kop": 1})", "\"name\""},
          Case{R"({"name": "С", "unit": "шт", "price_kop": 1.5})", "целым числом копеек"},
          Case{R"({"name": "С", "unit": "шт", "price_kop": "100"})", "целым числом копеек"}, Case{R"({"name": "С", "unit": "шт"})", "\"price_kop\""},
          Case{R"({"id": -1, "name": "С", "unit": "шт", "price_kop": 1})", "\"id\""},
          Case{R"({"name": 5, "unit": "шт", "price_kop": 1})", "\"name\" должно быть строкой"}}) {
        SCOPED_TRACE(c.input);
        try {
            (void)product_from_json(json::parse(c.input));
            ADD_FAILURE() << "ожидалась ValidationError с причиной \"" << c.reason << "\"";
        } catch (ValidationError const &ex) {
            EXPECT_THAT(ex.what(), HasSubstr(c.reason));
        }
    }
}
