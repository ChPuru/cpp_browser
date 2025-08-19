#ifndef JAVASCRIPT_H
#define JAVASCRIPT_H

#include <string>
#include <vector>
#include "duktape.h"

namespace JS {

    class JSEngine {
    public:
        JSEngine();
        ~JSEngine();

        // Executes a script and returns true on success
        bool run_script(const std::string& script);

        // Get the logs produced by console.log
        const std::vector<std::string>& get_logs() const;

    private:
        duk_context* m_ctx;
        std::vector<std::string> m_logs;

        // This static function will be called from within Duktape
        static int native_console_log(duk_context* ctx);
    };

} // namespace JS

#endif // JAVASCRIPT_H