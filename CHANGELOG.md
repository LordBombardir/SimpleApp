# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-07-26

### Added
- Initial release of **SimpleApp** — cross-platform C++23 desktop GUI application built with the [Dear ImGui](https://github.com/ocornut/imgui) framework.
- Dual native window backend architecture supporting **Win32 API + Direct3D 11** on Windows and **GLFW + OpenGL 3.3** on Linux.
- Custom borderless TitleBar with native Windows 11 DWM features support.
- Interactive Dashboard with animated page scrolling, technology stack display (featuring programming languages and C++ libraries), and real-time system metrics (CPU, RAM, Disk) updated at 1000ms intervals.
- Embedded localization system (`en_US`, `ru_RU`) with automatic system language detection via `GetUserPreferredUILanguages` (Windows) and `LANG`/`LC_ALL` (POSIX).
- Interactive Settings modal window with theme selection, language switcher, and clickable social links.
- Automated CI/CD workflows for Windows (`x64-windows-static`) and Linux (`x64-linux`) using CMake, vcpkg, `lukka/get-cmake`, `lukka/run-vcpkg`, and automatic SHA-256 release checksum generation.
