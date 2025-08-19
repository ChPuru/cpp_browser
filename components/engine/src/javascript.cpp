#include "javascript.h"
#include <iostream>
#include <stdexcept>

namespace JS {

    // The C++ function that JavaScript's `console.log` will call
    int JSEngine::native_console_log(duk_context* ctx) {
        // Get the JSEngine instance from the Duktape context
        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, -1, "js_engine_ptr");
        JSEngine* engine = static_cast<JSEngine*>(duk_to_pointer(ctx, -1));
        duk_pop_2(ctx); // Pop stash and pointer

        if (engine) {
            std::string log_message;
            int arg_count = duk_get_top(ctx);
            for (int i = 0; i < arg_count; ++i) {
                log_message += duk_safe_to_string(ctx, i);
                if (i < arg_count - 1) {
                    log_message += " ";
                }
            }
            engine->m_logs.push_back(log_message);
        }
        return 0; // No return value for console.log
    }

    JSEngine::JSEngine() {
        m_ctx = duk_create_heap_default();
        if (!m_ctx) {
            throw std::runtime_error("Failed to create Duktape context");
        }

        // Store a pointer to this JSEngine instance in the Duktape global stash
        duk_push_global_stash(m_ctx);
        duk_push_pointer(m_ctx, this);
        duk_put_prop_string(m_ctx, -2, "js_engine_ptr");
        duk_pop(m_ctx);

        // Create the `console` object in JavaScript
        duk_push_global_object(m_ctx);
        duk_push_object(m_ctx); // Create new 'console' object
        duk_push_c_function(m_ctx, native_console_log, DUK_VARARGS);
        duk_put_prop_string(m_ctx, -2, "log"); // console.log = native_console_log
        duk_put_prop_string(m_ctx, -2, "console"); // global.console = console
        duk_pop(m_ctx);
    }

    JSEngine::~JSEngine() {
        if (m_ctx) {
            duk_destroy_heap(m_ctx);
        }
    }

    bool JSEngine::run_script(const std::string& script) {
        if (duk_peval_string(m_ctx, script.c_str()) != 0) {
            std::string error = duk_safe_to_string(m_ctx, -1);
            m_logs.push_back("JS Error: " + error);
            duk_pop(m_ctx);
            return false;
        }
        duk_pop(m_ctx); // Pop the success result
        return true;
    }

    const std::vector<std::string>& JSEngine::get_logs() const {
        return m_logs;
    }

} // namespace JS