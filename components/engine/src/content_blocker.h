#ifndef CONTENT_BLOCKER_H
#define CONTENT_BLOCKER_H

#include <string>
#include <vector>
#include <unordered_set>

namespace Engine {

    class ContentBlocker {
    public:
        // Loads a list of blocking rules. Each rule is a simple substring to match.
        void load_rules(const std::vector<std::string>& rules);

        // Checks if a given URL should be blocked.
        bool should_block(const std::string& url) const;

    private:
        std::unordered_set<std::string> m_rules;
    };

} // namespace Engine

#endif // CONTENT_BLOCKER_H