#include "network_process.h"
#include <iostream>
#include <optional>

namespace Net {

    NetworkProcess::NetworkProcess(std::shared_ptr<Engine::ContentBlocker> blocker)
        : m_blocker(blocker) {}

    std::optional<Resource> NetworkProcess::request(const std::string& url) {
        std::cout << "[Network] Requesting URL: " << url << std::endl;

        if (m_blocker->should_block(url)) {
            std::cout << "[Network] *** BLOCKED *** by Content Blocker." << std::endl;
            return std::nullopt;
        }

        std::cout << "[Network] Allowed." << std::endl;

        // --- SIMULATED FETCH ---
        if (url == "test.html") {
            return Resource{
                url,
                // --- THIS IS THE CORRECTED HTML ---
                // The blocked <img> tags have been removed, as if they
                // were never in the original source file.
                R"(
                    <div id="main">
                        <h1>Welcome!</h1>
                        <p>This page is safe.</p>
                    </div>
                )"
            };
        }

        // In a real browser, we would also check if other resources like images are blocked.
        // For now, we just return dummy data.
        return Resource{ url, "Some data" };
    }

} // namespace Net