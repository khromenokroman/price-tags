# Образ price-tags: сборка в debian:trixie, запуск в debian:trixie-slim от непривилегированного пользователя.
# Справочники - в томе /data, конфигурация docker/cfg.json. Пример запуска: docker-compose.yml.
FROM debian:trixie AS build
RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake libfmt-dev nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B /build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr \
    && cmake --build /build -j"$(nproc)" \
    && DESTDIR=/out cmake --install /build --strip

FROM debian:trixie-slim
RUN useradd --system --uid 10001 --home-dir /data --shell /usr/sbin/nologin price-tags \
    && install -d -o price-tags -g price-tags -m 0750 /data
COPY --from=build /out/usr/bin/price-tags /usr/bin/price-tags
COPY --from=build /out/usr/share/doc/price-tags /usr/share/doc/price-tags
COPY docker/cfg.json /etc/price-tags/cfg.json
USER price-tags
VOLUME /data
EXPOSE 8082
ENTRYPOINT ["/usr/bin/price-tags", "/etc/price-tags/cfg.json"]
