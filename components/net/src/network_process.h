#ifndef NETWORK_PROCESS_H
#define NETWORK_PROCESS_H

#include <string>
#include <memory>
#include <optional>
#include "content_blocker.h"
#include <cpr/cpr.h> // <-- ADD THIS LINE

namespace Net {

    struct Resource {
        std::string url;
        std::string data;
        std::string content_type;
    };

    class NetworkProcess {
    public:
        NetworkProcess(std::shared_ptr<Engine::ContentBlocker> blocker);
        std::optional<Resource> request(const std::string& url);

    private:
        std::shared_ptr<Engine::ContentBlocker> m_blocker;
    };

} // namespace Net

#endif // NETWORK_PROCESS_H