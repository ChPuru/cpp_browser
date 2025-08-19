#ifndef JAVASCRIPT_H
#define JAVASCRIPT_H

#include <string>
#include <vector>
#include "duktape.h"
#include "dom.h" // Include DOM header

namespace JS {

    class JSEngine {
    public:
        JSEngine();
        ~JSEngine();

        // Give the JS engine a pointer to the document root
        void set_document(DOM::Node* doc);

        bool run_script(const std::string& script);
        const std::vector<std::string>& get_logs() const;

    private:
        duk_context* m_ctx;
        std::vector<std::string> m_logs;
        DOM::Node* m_document = nullptr;

        // C++ functions that will be callable from JavaScript
        static int native_console_log(duk_context* ctx);
        static int native_get_element_by_id(duk_context* ctx);
    };

} // namespace JS

#endif // JAVASCRIPT_H