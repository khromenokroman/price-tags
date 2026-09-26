#pragma once
#include <string_view>

/**
 * @brief Возвращает HTML единственной страницы приложения.
 *
 * Страница статическая: стили и скрипт встроены, данные подгружаются через JSON API.
 *
 * @return HTML-код страницы.
 */
[[nodiscard]] std::string_view index_page();
