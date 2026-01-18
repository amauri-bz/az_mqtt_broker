#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <system_error>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>

#include "../include/az_connect_mgr.hpp"

namespace AzMqttBroker {

ConnectMgr::ConnectMgr() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) throw std::system_error(errno, std::generic_category());
}

void ConnectMgr::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void ConnectMgr::add_socket(int fd) {
    set_nonblocking(fd);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // Edge Triggered
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl: add");
    }
}

int ConnectMgr::accept_socket(int fd) {
    int client_fd = accept(fd, nullptr, nullptr);
    return client_fd;
}

std::vector<int> ConnectMgr::wait_for_events() {
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    
    std::vector<int> active_fds;
    for (int n = 0; n < nfds; ++n) {
        active_fds.push_back(events[n].data.fd);
    }
    return active_fds;
}

int ConnectMgr::setup_server_socket(int port) {
    // 1. Criação do Socket (IPv4, TCP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Falha ao criar o socket: " + std::string(strerror(errno)));
    }

    // 2. Configurar SO_REUSEADDR
    // Isso evita o erro "Address already in use" se você reiniciar o broker rapidamente
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd);
        throw std::runtime_error("Falha no setsockopt: " + std::string(strerror(errno)));
    }

    // 3. Definir como Não-Bloqueante (Obrigatório para o seu design com epoll)
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(server_fd);
        throw std::runtime_error("Falha ao definir non-blocking: " + std::string(strerror(errno)));
    }

    // 4. Bind (Vincular à porta)
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Escuta em todas as interfaces de rede
    address.sin_port = htons(port);       // Converte para Network Byte Order

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(server_fd);
        throw std::runtime_error("Falha no bind na porta " + std::to_string(port) + ": " + std::string(strerror(errno)));
    }

    // 5. Listen (Habilitar para aceitar conexões)
    // O backlog de 4096 é comum para brokers de alta performance
    if (listen(server_fd, 4096) < 0) {
        close(server_fd);
        throw std::runtime_error("Falha no listen: " + std::string(strerror(errno)));
    }

    return server_fd;
}

}
