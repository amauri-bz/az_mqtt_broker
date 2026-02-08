#include <iostream>
#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <optional>
#include <mutex>
#include "../include/az_db_mgr.hpp"

namespace AzMqttBroker {

void DbMgr::save_session(const std::string& clientId, int fd) {
    std::unique_lock lock(mtx);

    auto session = std::make_shared<ClientSession>();
    session->clientId = clientId;
    session->socket_fd = fd;

    sessions[clientId] = session;
    fd_to_clientId[fd] = clientId;
}

std::shared_ptr<ClientSession> DbMgr::get_session(const std::string& clientId) {
    std::shared_lock lock(mtx);
    auto it = sessions.find(clientId);
    return (it != sessions.end()) ? it->second : nullptr;
}

std::shared_ptr<ClientSession> DbMgr::get_session_by_fd(int fd) {
    std::shared_lock lock(mtx);
    auto it_id = fd_to_clientId.find(fd);
    if (it_id == fd_to_clientId.end()) return nullptr;

    auto it_sess = sessions.find(it_id->second);
    return (it_sess != sessions.end()) ? it_sess->second : nullptr;
}

void DbMgr::remove_session(int fd) {
    std::unique_lock lock(mtx);

    auto it_id = fd_to_clientId.find(fd);
    if (it_id == fd_to_clientId.end()) return;

    std::string clientId = it_id->second;

    unsubscribe_from_all_tree(&root, clientId);

    sessions.erase(clientId);
    fd_to_clientId.erase(it_id);
}

void DbMgr::unsubscribe_from_all_tree(TopicNode* node, const std::string& clientId) {
    node->subscribers.erase(clientId);
    for (auto& [name, child] : node->children) {
        unsubscribe_from_all_tree(child.get(), clientId);
    }
}

void DbMgr::subscribe(const std::string& clientId, const std::string& topicFilter) {
    std::lock_guard lock(mtx);
    auto levels = split_topic(topicFilter);
    TopicNode* current = &root;
    for (const auto& lvl : levels) {
        if (!current->children.contains(lvl)) {
            current->children[lvl] = std::make_unique<TopicNode>();
        }
        current = current->children[lvl].get();
    }
    current->subscribers.insert(clientId);
}

std::vector<std::string> DbMgr::split_topic(const std::string& topic) {
    std::vector<std::string> levels;
    std::stringstream ss(topic);
    std::string level;
    while (std::getline(ss, level, '/')) {
        levels.push_back(level);
    }
    return levels;
}

std::vector<std::string> DbMgr::get_subscribers_for_topic(const std::string& topic) {
    std::shared_lock lock(mtx);
    auto levels = split_topic(topic);
    std::set<std::string> found_subscribers;

    // Função recursiva para busca com wildcards
    std::function<void(TopicNode*, size_t)> collect = [&](TopicNode* node, size_t depth) {
        // 1. Se chegamos ao fim do tópico da mensagem, adicionamos os inscritos aqui
        if (depth == levels.size()) {
            found_subscribers.insert(node->subscribers.begin(), node->subscribers.end());
        }

        // 2. Tratar wildcard '#' (corresponde a tudo daqui para baixo)
        if (node->children.contains("#")) {
            auto& hash_node = node->children["#"];
            found_subscribers.insert(hash_node->subscribers.begin(), hash_node->subscribers.end());
        }

        if (depth < levels.size()) {
            // 3. Tratar Match Exato
            if (node->children.contains(levels[depth])) {
                collect(node->children[levels[depth]].get(), depth + 1);
            }

            // 4. Tratar wildcard '+' (corresponde a qualquer string neste nível)
            if (node->children.contains("+")) {
                collect(node->children["+"].get(), depth + 1);
            }
        }
    };

    collect(&root, 0);
    return {found_subscribers.begin(), found_subscribers.end()};
}

}