#include "javascript.h"
#include <iostream>
#include <stdexcept>
#include <functional>

namespace JS {

    DOM::Node* find_node_by_id(DOM::Node* node, const std::string& id) {
        if (!node) return nullptr;
        if (node->type == DOM::NodeType::Element) {
            auto it = node->element_data.attributes.find("id");
            if (it != node->element_data.attributes.end() && it->second == id) {
                return node;
            }
        }
        for (auto& child : node->children) {
            if (auto found = find_node_by_id(child.get(), id)) {
                return found;
            }
        }
        return nullptr;
    }

    int innerHTML_setter(duk_context* ctx) {
        const char* new_text = duk_require_string(ctx, 0);
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "\xff""node_ptr");
        DOM::Node* n = static_cast<DOM::Node*>(duk_require_pointer(ctx, -1));
        duk_pop_2(ctx);

        if (n) {
            n->children.clear();
            n->children.push_back(DOM::create_text_node(new_text));
        }
        
        return 0;
    }

    int JSEngine::native_get_element_by_id(duk_context* ctx) {
        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, -1, "js_engine_ptr");
        JSEngine* engine = static_cast<JSEngine*>(duk_to_pointer(ctx, -1));
        duk_pop_2(ctx);

        if (!engine || !engine->m_document) { return 0; }

        const char* id = duk_require_string(ctx, 0);
        DOM::Node* found_node = find_node_by_id(engine->m_document, id);

        if (!found_node) { return 0; }

        duk_push_object(ctx);
        duk_push_string(ctx, "innerHTML");
        duk_push_c_function(ctx, innerHTML_setter, 1);
        duk_push_pointer(ctx, found_node);
        duk_put_prop_string(ctx, -2, "\xff""node_ptr");
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_SETTER);

        return 1;
    }

    JSEngine::JSEngine() {
        m_ctx = duk_create_heap_default();
        if (!m_ctx) { throw std::runtime_error("Failed to create Duktape context"); }

        duk_push_global_stash(m_ctx);
        duk_push_pointer(m_ctx, this);
        duk_put_prop_string(m_ctx, -2, "js_engine_ptr");
        duk_pop(m_ctx);

        duk_push_global_object(m_ctx);
        
        duk_push_object(m_ctx);
        duk_push_c_function(m_ctx, native_console_log, DUK_VARARGS);
        duk_put_prop_string(m_ctx, -2, "log");
        duk_put_prop_string(m_ctx, -2, "console");

        duk_push_object(m_ctx);
        duk_push_c_function(m_ctx, native_get_element_by_id, 1);
        duk_put_prop_string(m_ctx, -2, "getElementById");
        duk_put_prop_string(m_ctx, -2, "document");

        duk_pop(m_ctx);
    }

    void JSEngine::set_document(DOM::Node* doc) {
        m_document = doc;
    }

    JSEngine::~JSEngine() { if (m_ctx) { duk_destroy_heap(m_ctx); } }

    bool JSEngine::run_script(const std::string& script) {
        if (duk_peval_string(m_ctx, script.c_str()) != 0) {
            std::string error = duk_safe_to_string(m_ctx, -1);
            m_logs.push_back("JS Error: " + error);
            duk_pop(m_ctx);
            return false;
        }
        duk_pop(m_ctx);
        return true;
    }

    const std::vector<std::string>& JSEngine::get_logs() const {
        return m_logs;
    }

    int JSEngine::native_console_log(duk_context* ctx) {
        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, -1, "js_engine_ptr");
        JSEngine* engine = static_cast<JSEngine*>(duk_to_pointer(ctx, -1));
        duk_pop_2(ctx);
        if (engine) {
            std::string log_message;
            int arg_count = duk_get_top(ctx);
            for (int i = 0; i < arg_count; ++i) {
                log_message += duk_safe_to_string(ctx, i);
                if (i < arg_count - 1) { log_message += " "; }
            }
            engine->m_logs.push_back(log_message);
        }
        return 0;
    }
}