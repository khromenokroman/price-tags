#!/bin/bash
# Проверка Docker-образа: запуск от непривилегированного пользователя, журнал в docker logs, запись
# справочников в том /data, быстрая остановка по docker stop, данные после пересоздания контейнера.
#
# Использование: tests/docker_run.sh <образ>
set -u
IMAGE=$1
PORT=${PORT:-18095}
NAME=price-tags-test-$$
VOLUME=price-tags-test-$$
fail=0
ok() { echo "ok    $*"; }
bad() { echo "FAIL  $*"; fail=1; }
cleanup() { docker rm -f "$NAME" >/dev/null 2>&1; docker volume rm -f "$VOLUME" >/dev/null 2>&1; }
trap cleanup EXIT
start() { docker run -d --name "$NAME" -p "127.0.0.1:$PORT:8080" -v "$VOLUME:/data" "$IMAGE" >/dev/null; }
wait_http() { for _ in $(seq 100); do curl -sf "http://127.0.0.1:$PORT/api/products" >/dev/null && return 0; sleep 0.1; done; return 1; }
API=http://127.0.0.1:$PORT/api

start
wait_http && ok "отвечает" || { bad "не отвечает"; docker logs "$NAME" | tail -5; exit 1; }
[ "$(docker exec "$NAME" id -u)" = 10001 ] && ok "работает не от root (uid 10001)" || bad "пользователь: $(docker exec "$NAME" id)"
docker logs "$NAME" 2>&1 | grep -q 'Запущен сервер' && ok "журнал в docker logs" || bad "журнал: $(docker logs "$NAME" 2>&1 | tail -3)"
curl -sf -X POST "$API/products" -d '{"name":"Хлеб белый","unit":"шт","price_kop":4590}' >/dev/null \
    && docker exec "$NAME" grep -q 'Хлеб белый' /data/catalog.json && ok "запись в /data/catalog.json" || bad "запись справочников"
curl -sf "http://127.0.0.1:$PORT/" | grep -q '<title>' && ok "веб-страница отдаётся" || bad "нет страницы"

begin=$(date +%s)
docker stop -t 10 "$NAME" >/dev/null
spent=$(($(date +%s) - begin))
code=$(docker inspect -f '{{.State.ExitCode}}' "$NAME")
[ "$spent" -lt 5 ] && [ "$code" = 0 ] && ok "docker stop: остановлен за ${spent} с, код 0" || bad "docker stop: ${spent} с, код $code"
docker rm "$NAME" >/dev/null

start
wait_http || bad "после пересоздания не отвечает"
curl -s "$API/products" | grep -q 'Хлеб белый' && ok "данные на месте после пересоздания контейнера" \
    || bad "после пересоздания: $(curl -s "$API/products")"
exit $fail
