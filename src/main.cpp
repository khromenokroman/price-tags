#include <signal.h>
#include <syslog.h>

#include <cstdlib>
#include <iostream>
#include <thread>

#include "config.hpp"
#include "web_server.hpp"

int main(int argc, char *argv[]) {
    try {
        std::string_view const cfg_path = argc > 1 ? argv[1] : "/etc/price-tags/cfg.json";
        auto const cfg = load_config(cfg_path);
        openlog("price-tags", LOG_PID | LOG_CONS, LOG_USER);
        setlogmask(LOG_UPTO(cfg.log_level));

        sigset_t signals;
        sigemptyset(&signals);
        sigaddset(&signals, SIGINT);
        sigaddset(&signals, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &signals, nullptr);

        WebServer server{cfg};
        std::jthread signal_thread{[&server, &signals] {
            int sig{};
            sigwait(&signals, &sig);
            server.stop();
        }};
        try {
            server.run();
        } catch (...) {
            pthread_kill(signal_thread.native_handle(), SIGTERM);
            throw;
        }
        pthread_kill(signal_thread.native_handle(), SIGTERM);
    } catch (std::exception const &ex) {
        std::cerr << "Ошибка во время выполнения: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
