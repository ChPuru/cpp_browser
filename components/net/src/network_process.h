#ifndef NETWORK_PROCESS_H
#define NETWORK_PROCESS_H

#include <string>
#include <memory>
#include <optional> // <-- ADD THIS LINE
#include "content_blocker.h"

namespace Net {

    // A simple struct to represent a downloaded resource
    struct Resource {
        std::string url;
        std::string data;
    };

    class NetworkProcess {
    public:
        // The NetworkProcess needs a reference to the content blocker
        NetworkProcess(std::shared_ptr<Engine::ContentBlocker> blocker);

        // Tries to fetch a resource. Returns an empty optional if blocked.
        // This line will now compile correctly.
        std::optional<Resource> request(const std::string& url);

    private:
        std::shared_ptr<Engine::ContentBlocker> m_blocker;
    };

} // namespace Net

#endif // NETWORK_PROCESS_H