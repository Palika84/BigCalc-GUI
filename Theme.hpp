#pragma once
// Futasidejü megjelenes (skin) valto, a Lang.hpp nyelvvaltojahoz hasonloan.
// A tenyleges Qt stilus (QSS + fontok) a main.cpp-ben van, ez a fajl csak
// az enumot es a bigcalc_theme.ini mentes/betoltes kodjat tartalmazza,
// hogy platformfüggetlen (Qt-mentes) maradjon, mint a tobbi motor-header.

#include <string>

enum class Theme { TerminalDark = 0, LightDashboard = 1, DarkFintech = 2 };

// Alapertelmezett: Terminal / Crypto Dark.
inline Theme g_theme = Theme::TerminalDark;

inline const wchar_t* themeIniName() { return L"bigcalc_theme.ini"; }

inline Theme themeFromCode(const std::string& code) {
    if (code == "LIGHT") return Theme::LightDashboard;
    if (code == "FINTECH") return Theme::DarkFintech;
    return Theme::TerminalDark;
}

inline const char* themeToCode(Theme t) {
    switch (t) {
    case Theme::LightDashboard: return "LIGHT";
    case Theme::DarkFintech: return "FINTECH";
    default: return "TERMINAL";
    }
}
