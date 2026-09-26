#include "api.hpp"

#include <fmt/format.h>
#include <syslog.h>

#include <charconv>
#include <functional>

namespace {

void send_json(httplib::Response &res, int status, nlohmann::json const &j) {
    res.status = status;
    res.set_header("Cache-Control", "no-store");
    res.set_content(j.dump(), "application/json; charset=utf-8");
}

void send_error(httplib::Response &res, int status, std::string_view text) { send_json(res, status, {{"error", text}}); }

/**
 * @brief Выполняет обработчик и превращает исключения в HTTP-ответ с кодом ошибки.
 */
void guarded(httplib::Request const &req, httplib::Response &res, std::function<void()> const &handler) {
    syslog(LOG_DEBUG, "Поступил запрос(%s %s) от %s:%d", req.method.c_str(), req.path.c_str(), req.remote_addr.c_str(), req.remote_port);
    try {
        handler();
    } catch (nlohmann::json::parse_error const &ex) {
        send_error(res, 400, fmt::format("Некорректный JSON: {}", ex.what()));
    } catch (ValidationError const &ex) {
        send_error(res, 400, ex.what());
    } catch (NotFoundError const &ex) {
        send_error(res, 404, ex.what());
    } catch (std::exception const &ex) {
        syslog(LOG_ERR, "Ошибка обработки запроса(%s %s): %s", req.method.c_str(), req.path.c_str(), ex.what());
        send_error(res, 500, ex.what());
    }
}

std::uint64_t path_id(httplib::Request const &req) {
    auto const s = req.matches[1].str();
    std::uint64_t id{0};
    auto const [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), id);
    if (ec != std::errc{} || ptr != s.data() + s.size() || id == 0) {
        throw NotFoundError(fmt::format("Некорректный id \"{}\"", s));
    }
    return id;
}

/**
 * @brief Регистрирует GET/POST/PUT/DELETE для одного справочника.
 * @tparam T Тип записи (Organization или Product).
 */
template <typename T>
void register_crud(httplib::Server &server, std::string const &path, std::function<std::vector<T>()> list, std::function<T(T)> add,
                   std::function<T(T)> update, std::function<void(std::uint64_t)> remove, std::function<T(nlohmann::json const &)> from_json) {
    auto const item_path = path + R"(/(\d+))";

    server.Get(path, [list](httplib::Request const &req, httplib::Response &res) { guarded(req, res, [&] { send_json(res, 200, list()); }); });
    server.Post(path, [add, from_json](httplib::Request const &req, httplib::Response &res) {
        guarded(req, res, [&] {
            auto item = from_json(nlohmann::json::parse(req.body));
            send_json(res, 201, add(std::move(item)));
        });
    });
    server.Put(item_path, [update, from_json](httplib::Request const &req, httplib::Response &res) {
        guarded(req, res, [&] {
            auto const id = path_id(req);
            auto item = from_json(nlohmann::json::parse(req.body));
            item.id = id;
            send_json(res, 200, update(std::move(item)));
        });
    });
    server.Delete(item_path, [remove](httplib::Request const &req, httplib::Response &res) {
        guarded(req, res, [&] {
            remove(path_id(req));
            res.status = 204;
        });
    });
}

} // namespace

void register_api(httplib::Server &server, Catalog &catalog) {
    register_crud<Organization>(
        server, "/api/organizations", [&catalog] { return catalog.organizations(); },
        [&catalog](Organization o) { return catalog.add_organization(std::move(o)); },
        [&catalog](Organization o) { return catalog.update_organization(std::move(o)); },
        [&catalog](std::uint64_t id) { catalog.remove_organization(id); }, organization_from_json);
    register_crud<Product>(
        server, "/api/products", [&catalog] { return catalog.products(); }, [&catalog](Product p) { return catalog.add_product(std::move(p)); },
        [&catalog](Product p) { return catalog.update_product(std::move(p)); }, [&catalog](std::uint64_t id) { catalog.remove_product(id); },
        product_from_json);
}
