#pragma once
#include <cstdint>
#include <mutex>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief Организация: её название печатается в заголовке ценника.
 */
struct Organization {
    std::string name{};  ///< Название.
    std::uint64_t id{0}; ///< Идентификатор, назначается справочником.
};

/**
 * @brief Товар справочника.
 */
struct Product {
    std::string name{};        ///< Наименование.
    std::string unit{};        ///< Единица измерения (шт, кг, л).
    std::string article{};     ///< Артикул, необязательный.
    std::int64_t price_kop{0}; ///< Цена в копейках.
    std::uint64_t id{0};       ///< Идентификатор, назначается справочником.
};

/**
 * @brief Запись справочника с указанным идентификатором не найдена.
 */
class NotFoundError : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Данные не прошли проверку (пустое название, отрицательная цена и т.п.).
 */
class ValidationError : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Справочники организаций и товаров с хранением в JSON-файле.
 *
 * Каждое изменение сохраняется на диск атомарно: запись во временный файл, fsync, rename.
 * Если сохранить не удалось, состояние в памяти не меняется. Методы потокобезопасны.
 */
class Catalog {
   public:
    /// Максимальная длина текстовых полей в байтах.
    static constexpr std::size_t MAX_TEXT_LEN = 256;
    /// Максимальная цена в копейках (10 млрд рублей).
    static constexpr std::int64_t MAX_PRICE_KOP = 1'000'000'000'000;

    /**
     * @brief Загружает справочники из файла. Если файла нет, справочники пусты.
     * @param path Путь к файлу справочников.
     * @throw std::runtime_error если файл не читается или содержит ошибки.
     */
    explicit Catalog(std::string path);

    /**
     * @brief Возвращает все организации в порядке добавления.
     */
    [[nodiscard]] std::vector<Organization> organizations() const;

    /**
     * @brief Добавляет организацию.
     * @param org Данные организации; поле id игнорируется.
     * @return Добавленная организация с назначенным id.
     * @throw ValidationError при некорректных данных.
     * @throw std::runtime_error если не удалось сохранить файл.
     */
    Organization add_organization(Organization org);

    /**
     * @brief Изменяет организацию с идентификатором org.id.
     * @return Изменённая организация.
     * @throw NotFoundError если организации нет.
     * @throw ValidationError при некорректных данных.
     * @throw std::runtime_error если не удалось сохранить файл.
     */
    Organization update_organization(Organization org);

    /**
     * @brief Удаляет организацию.
     * @throw NotFoundError если организации нет.
     * @throw std::runtime_error если не удалось сохранить файл.
     */
    void remove_organization(std::uint64_t id);

    /**
     * @brief Возвращает все товары в порядке добавления.
     */
    [[nodiscard]] std::vector<Product> products() const;

    /**
     * @brief Добавляет товар.
     * @param product Данные товара; поле id игнорируется.
     * @return Добавленный товар с назначенным id.
     * @throw ValidationError при некорректных данных.
     * @throw std::runtime_error если не удалось сохранить файл.
     */
    Product add_product(Product product);

    /**
     * @brief Изменяет товар с идентификатором product.id.
     * @return Изменённый товар.
     * @throw NotFoundError если товара нет.
     * @throw ValidationError при некорректных данных.
     * @throw std::runtime_error если не удалось сохранить файл.
     */
    Product update_product(Product product);

    /**
     * @brief Удаляет товар.
     * @throw NotFoundError если товара нет.
     * @throw std::runtime_error если не удалось сохранить файл.
     */
    void remove_product(std::uint64_t id);

   private:
    /**
     * @brief Состояние справочников, которое целиком сохраняется в файл.
     */
    struct State {
        std::vector<Organization> organizations{}; ///< Организации.
        std::vector<Product> products{};           ///< Товары.
        std::uint64_t next_id{1};                  ///< Следующий свободный идентификатор.
    };

    /**
     * @brief Сохраняет состояние в файл и, если запись удалась, делает его текущим.
     * @throw std::runtime_error если не удалось сохранить файл.
     */
    void commit(State state);

    State m_state;              // 56
    mutable std::mutex m_mutex; // 40
    std::string m_path;         // 32
};

/**
 * @brief Преобразует организацию в JSON.
 */
void to_json(nlohmann::json &j, Organization const &org);

/**
 * @brief Преобразует товар в JSON.
 */
void to_json(nlohmann::json &j, Product const &product);

/**
 * @brief Читает организацию из JSON. Поле id необязательно (0, если нет).
 * @throw ValidationError если поля имеют неверный тип.
 */
[[nodiscard]] Organization organization_from_json(nlohmann::json const &j);

/**
 * @brief Читает товар из JSON. Поля id и article необязательны.
 * @throw ValidationError если поля имеют неверный тип или отсутствуют.
 */
[[nodiscard]] Product product_from_json(nlohmann::json const &j);
