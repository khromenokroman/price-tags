#include "index_page.hpp"

namespace {

constexpr std::string_view PAGE = R"HTML(<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Ценники</title>
</head>
<body>
<h1>Ценники</h1>
</body>
</html>
)HTML";

} // namespace

std::string_view index_page() { return PAGE; }
