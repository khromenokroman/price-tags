#include "catalog.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <set>
#include <system_error>

namespace {

constexpr int FILE_VERSION = 1;

std::string trim(std::string_view s) {
    auto const first = s.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        return {};
    }
    auto const last = s.find_last_not_of(" \t\r\n");
    return std::string{s.substr(first, last - first + 1)};
}

std::string read_string(nlohmann::json const &j, std::string_view key, bool required) {
    if (!j.contains(key)) {
        if (required) {
            throw ValidationError(fmt::format("Не указано поле \"{}\"", key));
        }
        return {};
    }
    if (!j.at(key).is_string()) {
        throw ValidationError(fmt::format("Поле \"{}\" должно быть строкой", key));
    }
    return j.at(key).get<std::string>();
}

std::uint64_t read_id(nlohmann::json const &j) {
    if (!j.contains("id")) {
        return 0;
    }
    if (!j.at("id").is_number_unsigned()) {
        throw ValidationError("Поле \"id\" должно быть положительным целым числом");
    }
    return j.at("id").get<std::uint64_t>();
}

std::string checked_text(std::string_view value, std::string_view what, bool required) {
    auto text = trim(value);
    if (required && text.empty()) {
        throw ValidationError(fmt::format("{}: значение не может быть пустым", what));
    }
    if (text.size() > Catalog::MAX_TEXT_LEN) {
        throw ValidationError(fmt::format("{}: длина {} байт превышает {}", what, text.size(), Catalog::MAX_TEXT_LEN));
    }
    return text;
}

Organization checked(Organization org) {
    org.name = checked_text(org.name, "Название организации", true);
    return org;
}

Product checked(Product p) {
    p.name = checked_text(p.name, "Наименование товара", true);
    p.unit = checked_text(p.unit, "Единица измерения", true);
    p.article = checked_text(p.article, "Артикул", false);
    if (p.price_kop < 0 || p.price_kop > Catalog::MAX_PRICE_KOP) {
        throw ValidationError(fmt::format("Цена {} коп. вне допустимого диапазона 0-{}", p.price_kop, Catalog::MAX_PRICE_KOP));
    }
    return p;
}

template <typename T>
typename std::vector<T>::iterator find_id(std::vector<T> &items, std::uint64_t id, std::string_view what) {
    auto it = std::find_if(items.begin(), items.end(), [id](T const &item) { return item.id == id; });
    if (it == items.end()) {
        throw NotFoundError(fmt::format("{} (id {})", what, id));
    }
    return it;
}

[[noreturn]] void throw_errno(std::string_view action, std::string_view path) {
    auto const err = errno;
    throw std::runtime_error(fmt::format("Не удалось {}({}): {}", action, path, std::generic_category().message(err)));
}

void write_all(int fd, std::string_view data, std::string_view path) {
    while (!data.empty()) {
        auto const n = ::write(fd, data.data(), data.size());
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw_errno("записать файл", path);
        }
        data.remove_prefix(static_cast<std::size_t>(n));
    }
}

/**
 * @brief Атомарно заменяет содержимое файла: временный файл, fsync, rename, fsync каталога.
 */
void write_file_atomic(std::string const &path, std::string_view data) {
    auto const tmp = path + ".tmp";
    int const fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd < 0) {
        throw_errno("создать файл", tmp);
    }
    try {
        write_all(fd, data, tmp);
        if (::fsync(fd) != 0) {
            throw_errno("сбросить на диск файл", tmp);
        }
    } catch (...) {
        ::close(fd);
        ::unlink(tmp.c_str());
        throw;
    }
    if (::close(fd) != 0) {
        ::unlink(tmp.c_str());
        throw_errno("закрыть файл", tmp);
    }
    if (::rename(tmp.c_str(), path.c_str()) != 0) {
        ::unlink(tmp.c_str());
        throw_errno("переименовать файл", tmp);
    }
    auto dir = std::filesystem::path{path}.parent_path().string();
    if (dir.empty()) {
        dir = ".";
    }
    int const dfd = ::open(dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dfd >= 0) {
        ::fsync(dfd);
        ::close(dfd);
    }
}

template <typename T>
void check_loaded_ids(std::vector<T> const &items, std::set<std::uint64_t> &seen, std::uint64_t next_id, std::string_view path) {
    for (auto const &item : items) {
        if (item.id == 0 || item.id >= next_id || !seen.insert(item.id).second) {
            throw std::runtime_error(fmt::format("Ошибка в справочниках({}): некорректный или повторяющийся id {}", path, item.id));
        }
    }
}

} // namespace

void to_json(nlohmann::json &j, Organization const &org) { j = nlohmann::json{{"id", org.id}, {"name", org.name}}; }

void to_json(nlohmann::json &j, Product const &product) {
    j = nlohmann::json{
        {"id", product.id}, {"name", product.name}, {"unit", product.unit}, {"article", product.article}, {"price_kop", product.price_kop}};
}

Organization organization_from_json(nlohmann::json const &j) {
    if (!j.is_object()) {
        throw ValidationError("Организация должна быть JSON-объектом");
    }
    Organization org;
    org.id = read_id(j);
    org.name = read_string(j, "name", true);
    return org;
}

Product product_from_json(nlohmann::json const &j) {
    if (!j.is_object()) {
        throw ValidationError("Товар должен быть JSON-объектом");
    }
    Product p;
    p.id = read_id(j);
    p.name = read_string(j, "name", true);
    p.unit = read_string(j, "unit", true);
    p.article = read_string(j, "article", false);
    if (!j.contains("price_kop")) {
        throw ValidationError("Не указано поле \"price_kop\"");
    }
    if (!j.at("price_kop").is_number_integer()) {
        throw ValidationError("Поле \"price_kop\" должно быть целым числом копеек");
    }
    p.price_kop = j.at("price_kop").get<std::int64_t>();
    return p;
}

Catalog::Catalog(std::string path) : m_path{std::move(path)} {
    std::error_code ec;
    if (!std::filesystem::exists(m_path, ec) && !ec) {
        return;
    }
    std::ifstream file{m_path};
    if (!file.is_open()) {
        throw_errno("открыть справочники", m_path);
    }
    try {
        auto const j = nlohmann::json::parse(file);
        if (!j.is_object() || j.value("version", 0) != FILE_VERSION) {
            throw std::runtime_error(fmt::format("ожидается объект с \"version\": {}", FILE_VERSION));
        }
        State state;
        state.next_id = j.at("next_id").get<std::uint64_t>();
        for (auto const &o : j.at("organizations")) {
            state.organizations.push_back(checked(organization_from_json(o)));
        }
        for (auto const &p : j.at("products")) {
            state.products.push_back(checked(product_from_json(p)));
        }
        std::set<std::uint64_t> seen;
        check_loaded_ids(state.organizations, seen, state.next_id, m_path);
        check_loaded_ids(state.products, seen, state.next_id, m_path);
        m_state = std::move(state);
    } catch (std::exception const &ex) {
        throw std::runtime_error(fmt::format("Ошибка разбора справочников({}): {}", m_path, ex.what()));
    }
}

void Catalog::commit(State state) {
    nlohmann::json const j{
        {"version", FILE_VERSION}, {"next_id", state.next_id}, {"organizations", state.organizations}, {"products", state.products}};
    write_file_atomic(m_path, j.dump(2) + "\n");
    m_state = std::move(state);
}

std::vector<Organization> Catalog::organizations() const {
    std::scoped_lock lock{m_mutex};
    return m_state.organizations;
}

Organization Catalog::add_organization(Organization org) {
    org = checked(std::move(org));
    std::scoped_lock lock{m_mutex};
    auto state = m_state;
    org.id = state.next_id++;
    state.organizations.push_back(org);
    commit(std::move(state));
    return org;
}

Organization Catalog::update_organization(Organization org) {
    org = checked(std::move(org));
    std::scoped_lock lock{m_mutex};
    auto state = m_state;
    *find_id(state.organizations, org.id, "Организация не найдена") = org;
    commit(std::move(state));
    return org;
}

void Catalog::remove_organization(std::uint64_t id) {
    std::scoped_lock lock{m_mutex};
    auto state = m_state;
    state.organizations.erase(find_id(state.organizations, id, "Организация не найдена"));
    commit(std::move(state));
}

std::vector<Product> Catalog::products() const {
    std::scoped_lock lock{m_mutex};
    return m_state.products;
}

Product Catalog::add_product(Product product) {
    product = checked(std::move(product));
    std::scoped_lock lock{m_mutex};
    auto state = m_state;
    product.id = state.next_id++;
    state.products.push_back(product);
    commit(std::move(state));
    return product;
}

Product Catalog::update_product(Product product) {
    product = checked(std::move(product));
    std::scoped_lock lock{m_mutex};
    auto state = m_state;
    *find_id(state.products, product.id, "Товар не найден") = product;
    commit(std::move(state));
    return product;
}

void Catalog::remove_product(std::uint64_t id) {
    std::scoped_lock lock{m_mutex};
    auto state = m_state;
    state.products.erase(find_id(state.products, id, "Товар не найден"));
    commit(std::move(state));
}
