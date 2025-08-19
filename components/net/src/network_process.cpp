#include "network_process.h"
#include <iostream>

namespace Net {

    NetworkProcess::NetworkProcess(std::shared_ptr<Engine::ContentBlocker> blocker)
        : m_blocker(blocker) {}

    std::optional<Resource> NetworkProcess::request(const std::string& url) {
        std::cout << "[Network] Requesting URL: " << url << std::endl;

        if (m_blocker->should_block(url)) {
            std::cout << "[Network] *** BLOCKED *** by Content Blocker." << std::endl;
            return std::nullopt;
        }

        // --- REAL HTTP REQUEST ---
        cpr::Response r = cpr::Get(cpr::Url{url});

        if (r.status_code == 200) {
            std::cout << "[Network] Success (" << r.status_code << ") [" << r.header["content-type"] << "]" << std::endl;
            return Resource{
                url,
                r.text,
                r.header["content-type"]
            };
        } else {
            std::cout << "[Network] !!! FAILED !!! (" << r.status_code << ")" << std::endl;
            // Return a simple error page
            return Resource{
                url,
                "<h1>Error " + std::to_string(r.status_code) + "</h1>",
                "text/html"
            };
        }
    }

} // namespace Net