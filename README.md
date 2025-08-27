# Netscape Matrix

A web browser engine and graphical UI built from scratch in C++, themed with retro-futurism. This project is a deep dive into the core components of the web, from parsing and layout to rendering and JavaScript execution.

## About The Project

This browser was built as a learning exercise to understand the technologies that power the modern web. It combines a custom-built rendering engine with a unique "Netscape Navigator meets The Matrix" theme.

The entire development journey was guided by an AI assistant, showcasing a modern collaborative approach to complex software development.

## ✅ Features

The browser includes a sophisticated engine and a feature-rich UI, all built from the ground up:

**Engine:**

* **HTML Parser:** Parses "tag soup" HTML into a Document Object Model (DOM) tree.
* **CSS Parser:** Parses CSS into a stylesheet, understanding selectors, properties, and values (keywords, colors, pixels).
* **Style Engine:** Creates a Style Tree by applying CSS rules to the DOM, including property inheritance.
* **Layout Engine:**

  * Implements the full **CSS Box Model** (`padding`, `margin`).
  * Supports a simplified version of **CSS Flexbox** (`display: flex`, `justify-content`).

* **JavaScript Engine:**
  * Integrates the Duktape JS engine to execute scripts.
  * Provides a `console.log` implementation.
  * 1Features a basic **DOM API** with `document.getElementById` that can modify page content.

**UI & Features:**

* **Graphical Interface:** A hardware-accelerated UI built with GLFW, GLAD, and ImGui.
* **Custom Theming:** A unique "Netscape + Matrix" theme.
* **Real Networking:** Fetches live websites using the `cpr` HTTP library.
* **Content Blocker:** A basic privacy-respecting ad/tracker blocker.
* **Tabbed Browsing:** A functional tab bar.
* **Bookmarks:** A working bookmarks menu for quick navigation.
* **Developer Console:** An interactive console to view logs and execute JavaScript.

## 🚀 Building from Source

### Prerequisites

1. **C++ Compiler:** A modern C++ compiler (MSVC on Windows, GCC/Clang on Linux/macOS).
2. **CMake:** Version 3.20 or higher.
3. **vcpkg:** The C++ package manager from Microsoft. Ensure it is installed and integrated (`vcpkg integrate install`).

### Build Steps

1. **Clone the repository:**

    ```bash
    git clone https://github.com/ChPuru/cpp_browser.git
    cd cpp_browser
    ```

2. **Install dependencies:** `vcpkg` will handle this automatically when you run CMake, but you can also do it manually.

    ```bash
    # This step is optional, CMake will do it for you.
    vcpkg install
    ```

3. **Configure the project with CMake:**
    * Create a build directory: `mkdir build && cd build`
    * Run CMake, pointing it to your vcpkg toolchain file. **Replace `C:/vcpkg` with your actual vcpkg path.**

    ```bash
    cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
    ```

4. **Build the project:**

    ```bash
    cmake --build .
    ```

5. **Run the browser:**

    ```bash
    ./components/ui/Debug/browser.exe
    ```

## License

Distributed under the MIT License. See `LICENSE` for more information.
