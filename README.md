[![English](https://custom-icon-badges.demolab.com/badge/-English-green?style=for-the-badge)](README.md) [![Russian](https://custom-icon-badges.demolab.com/badge/-Russian-gray?style=for-the-badge)](README.ru.md)

# What is SimpleApp?
**SimpleApp** is a high-performance, cross-platform C++23 desktop GUI application built with the [Dear ImGui](https://github.com/ocornut/imgui) framework, featuring a modern custom interface, live system metrics monitoring, dynamic localization, and an embedded resource system.

# Features & Management
The application offers an interactive dashboard with smooth page scrolling and intuitive UI controls:
- **Custom TitleBar**: A custom TitleBar supporting native Windows 11 DWM features.
- **Real-Time System Metrics**: Hardware monitoring cards providing live CPU usage, RAM statistics, and disk metrics sampled at 1000ms intervals.
- **Embedded Localization**: Dynamic language switcher (`en_US` / `ru_RU`) with system language auto-detection via `GetUserPreferredUILanguages` on Windows and `LANG` environment variables on POSIX.
