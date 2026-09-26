#pragma once
#include <httplib.h>

#include "catalog.hpp"

/**
 * @brief Регистрирует маршруты JSON API справочников.
 *
 * Для организаций (/api/organizations) и товаров (/api/products):
 * GET список, POST добавление (201), PUT /{id} изменение, DELETE /{id} удаление (204).
 * Ошибки возвращаются как {"error": "текст"}: 400 некорректные данные, 404 нет записи, 500 сбой сохранения.
 *
 * @param server HTTP-сервер.
 * @param catalog Справочники; должны жить дольше сервера.
 */
void register_api(httplib::Server &server, Catalog &catalog);
