#include "content_blocker.h"

namespace Engine {

    void ContentBlocker::load_rules(const std::vector<std::string>& rules) {
        for (const auto& rule : rules) {
            m_rules.insert(rule);
        }
    }

    bool ContentBlocker::should_block(const std::string& url) const {
        // This is a very simple implementation. A real blocker uses more complex rules.
        // We check if any of our rule strings appear as a substring in the URL.
        for (const auto& rule : m_rules) {
            if (url.find(rule) != std::string::npos) {
                return true; // Found a match, should be blocked.
            }
        }
        return false; // No matches, allowed.
    }

} // namespace Engine