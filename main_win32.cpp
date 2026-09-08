#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <fstream>
#include <string>
#include <vector>

#include "Calculator.hpp"
#include "Lang.hpp"
#include "Theme.hpp"
#include "ModeEngine.hpp"
#include "crypto/BitcoinCrypto.hpp"

#pragma comment(lib, "comctl32.lib")

#ifndef LVS_EX_FULLROWSELECT
#define LVS_EX_FULLROWSELECT 0x00000020
#endif
#ifndef LVS_EX_GRIDLINES
#define LVS_EX_GRIDLINES 0x00000001
#endif
#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0x00010000
#endif
#ifndef LVS_EX_LABELTIP
#define LVS_EX_LABELTIP 0x00004000
#endif

#include <cstring>

enum CtrlId : int {
    ID_EDIT_LEFT = 1001,
    ID_COMBO_OP,
    ID_EDIT_RIGHT,
    ID_EDIT_PREC,
    ID_EDIT_EXPR,
    ID_BTN_CALC,
    ID_BTN_CLEAR,
    ID_EDIT_DEC,
    ID_EDIT_HEX,
    ID_LIST_HIST = 1020,
    ID_LBL_LEFT,
    ID_LBL_OP,
    ID_LBL_RIGHT,
    ID_LBL_PREC,
    ID_LBL_EXPR,
    ID_LBL_DEC,
    ID_LBL_HEX,
    ID_LBL_HIST,
    ID_STATUS,
    ID_SPLIT_DEC_HEX,
    ID_SPLIT_HEX_HIST,
    ID_LBL_LANG,
    ID_COMBO_LANG,
    ID_LBL_THEME,
    ID_COMBO_THEME,
    ID_LBL_APPTITLE,
    ID_LBL_APPSUBTITLE,
    ID_CTX_COPY_CELL = 2001,
    ID_CTX_COPY_ROW
};

enum : int {
    SPLIT_THICK = 6,
    PANEL_MIN_H = 40,
    CARD_PAD = 6
};

// ---------------------------------------------------------------
// Megjelenes (skin) tamogatas: szinpaletta + betutipus-parositas
// temankent. A Qt6-os (Linux) build QSS temainak Win32/GDI megfeleloi.
// ---------------------------------------------------------------

struct ThemePalette {
    COLORREF winBg;
    COLORREF panelBg;
    COLORREF panelBorder;
    COLORREF text;
    COLORREF textDim;
    COLORREF accent;
    COLORREF accentText;
    COLORREF inputBg;
    COLORREF btnSecondaryText;
    COLORREF tableHeaderBg;
    COLORREF tableHeaderText;
    COLORREF tableRowBg;
    COLORREF tableSelBg;
    COLORREF tableSelText;
    const wchar_t* uiFont;
    const wchar_t* uiFontTitle;
    const wchar_t* monoFont;
};

static const ThemePalette& paletteFor(Theme t) {
    static const ThemePalette terminalDark{
        RGB(0x17, 0x1d, 0x1a), // winBg
        RGB(0x10, 0x15, 0x13), // panelBg
        RGB(0x50, 0x65, 0x52), // panelBorder (blended)
        RGB(0xcf, 0xea, 0xd9), // text
        RGB(0x6f, 0x8f, 0x7c), // textDim
        RGB(0x39, 0xff, 0x88), // accent
        RGB(0x0c, 0x13, 0x10), // accentText
        RGB(0x10, 0x15, 0x13), // inputBg
        RGB(0x9d, 0xb8, 0xa9), // btnSecondaryText
        RGB(0x1c, 0x26, 0x22), // tableHeaderBg
        RGB(0x9d, 0xb8, 0xa9), // tableHeaderText
        RGB(0x10, 0x15, 0x13), // tableRowBg
        RGB(0x2a, 0x5b, 0x3f), // tableSelBg
        RGB(0xea, 0xff, 0xf2), // tableSelText
        L"JetBrains Mono", L"JetBrains Mono", L"JetBrains Mono"
    };
    static const ThemePalette lightDashboard{
        RGB(0xf6, 0xf7, 0xfb), // winBg
        RGB(0xff, 0xff, 0xff), // panelBg
        RGB(0xdd, 0xe0, 0xea), // panelBorder
        RGB(0x2c, 0x30, 0x42), // text
        RGB(0x66, 0x6c, 0x80), // textDim
        RGB(0x4f, 0x5f, 0xd1), // accent
        RGB(0xff, 0xff, 0xff), // accentText
        RGB(0xff, 0xff, 0xff), // inputBg
        RGB(0x4b, 0x51, 0x64), // btnSecondaryText
        RGB(0xf6, 0xf7, 0xfb), // tableHeaderBg
        RGB(0x6a, 0x6f, 0x82), // tableHeaderText
        RGB(0xff, 0xff, 0xff), // tableRowBg
        RGB(0xe8, 0xea, 0xff), // tableSelBg
        RGB(0x2c, 0x30, 0x42), // tableSelText
        L"Manrope", L"Manrope", L"IBM Plex Mono"
    };
    static const ThemePalette darkFintech{
        RGB(0x1b, 0x21, 0x30), // winBg
        RGB(0x22, 0x29, 0x39), // panelBg
        RGB(0x33, 0x3c, 0x50), // panelBorder
        RGB(0xdd, 0xe1, 0xea), // text
        RGB(0x92, 0x9a, 0xad), // textDim
        RGB(0xe0, 0xb2, 0x5a), // accent
        RGB(0x1b, 0x21, 0x30), // accentText
        RGB(0x22, 0x29, 0x39), // inputBg
        RGB(0xa7, 0xad, 0xba), // btnSecondaryText
        RGB(0x26, 0x2e, 0x40), // tableHeaderBg
        RGB(0x92, 0x9a, 0xad), // tableHeaderText
        RGB(0x22, 0x29, 0x39), // tableRowBg
        RGB(0x4a, 0x3c, 0x22), // tableSelBg
        RGB(0xf3, 0xe6, 0xc8), // tableSelText
        L"Space Grotesk", L"Space Grotesk", L"IBM Plex Mono"
    };
    switch (t) {
    case Theme::LightDashboard: return lightDashboard;
    case Theme::DarkFintech: return darkFintech;
    default: return terminalDark;
    }
}

struct AppState {
    HWND hwnd = nullptr;
    HWND hLblAppTitle = nullptr;
    HWND hLblAppSubtitle = nullptr;
    HWND hLeft = nullptr;
    HWND hOp = nullptr;
    HWND hRight = nullptr;
    HWND hPrec = nullptr;
    HWND hExpr = nullptr;
    HWND hBtnCalc = nullptr;
    HWND hBtnClear = nullptr;
    HWND hDec = nullptr;
    HWND hHex = nullptr;
    HWND hHist = nullptr;
    HWND hStatus = nullptr;
    HWND hSplitDecHex = nullptr;
    HWND hSplitHexHist = nullptr;
    HWND hLblLang = nullptr;
    HWND hComboLang = nullptr;
    HWND hLblTheme = nullptr;
    HWND hComboTheme = nullptr;
    HFONT hFontTitle = nullptr;
    HFONT hFontUi = nullptr;
    HFONT hFontUiSemibold = nullptr;
    HFONT hFontMono = nullptr;
    HBRUSH hBgBrush = nullptr;
    HBRUSH hInputBrush = nullptr;
    bool ready = false;
    // A harom also panel relativ magassaga (osszeg = 1.0)
    double fracDec = 0.36;
    double fracHex = 0.36;
    double fracHist = 0.28;
    int dragSplit = 0; // 0=nincs, 1=dec/hex, 2=hex/hist
    int clientW = 0;
    int clientH = 0;
    int yDecTop = 0;
    int yHexTop = 0;
    int yHistTop = 0;
    int hDecPx = 0;
    int hHexPx = 0;
    int hHistPx = 0;
    int histRow = -1;
    int histCol = 0;
    RECT rcDecCard{};
    RECT rcHexCard{};
    RECT rcHistCard{};
};

static AppState g;
static WNDPROC g_origHeaderProc = nullptr;

static std::wstring widen(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n ? n - 1 : 0, L'\0');
    if (n > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    return w;
}

static std::string narrow(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n ? n - 1 : 0, '\0');
    if (n > 1) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    return s;
}

static std::wstring getText(HWND h) {
    int len = GetWindowTextLengthW(h);
    std::wstring s(len, L'\0');
    if (len > 0) GetWindowTextW(h, &s[0], len + 1);
    return s;
}

static void setText(HWND h, const std::wstring& s) {
    SetWindowTextW(h, s.c_str());
}

static void setStatus(const std::wstring& s) {
    if (g.hStatus) setText(g.hStatus, s);
}

// --- exe konyvtar, nyelvi/tema beallitas fajlok es becsomagolt betutipusok ---
static std::wstring exeDir() {
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::wstring path(buf, n);
    size_t slash = path.find_last_of(L"\\/");
    return (slash == std::wstring::npos) ? L"" : path.substr(0, slash + 1);
}

static std::wstring exeDirIniPath(const wchar_t* name) {
    return exeDir() + name;
}

static void loadLangFromIni() {
    std::ifstream in(exeDirIniPath(langIniName()).c_str());
    if (!in) return;
    std::string code;
    std::getline(in, code);
    while (!code.empty() && (code.back() == '\r' || code.back() == '\n')) code.pop_back();
    g_lang = langFromCode(code);
}

static void saveLangToIni() {
    std::ofstream out(exeDirIniPath(langIniName()).c_str(), std::ios::trunc);
    if (!out) return;
    out << langToCode(g_lang);
}

static void loadThemeFromIni() {
    std::ifstream in(exeDirIniPath(themeIniName()).c_str());
    if (!in) return;
    std::string code;
    std::getline(in, code);
    while (!code.empty() && (code.back() == '\r' || code.back() == '\n')) code.pop_back();
    g_theme = themeFromCode(code);
}

static void saveThemeToIni() {
    std::ofstream out(exeDirIniPath(themeIniName()).c_str(), std::ios::trunc);
    if (!out) return;
    out << themeToCode(g_theme);
}

// A becsomagolt betutipusok az exe melletti fonts\ konyvtarban keresendok
// (a build.bat/telepites masolja oda a repo fonts/ mappajat).
static void loadBundledFonts() {
    const wchar_t* files[] = {
        L"fonts\\JetBrainsMono\\JetBrainsMono-Regular.ttf",
        L"fonts\\JetBrainsMono\\JetBrainsMono-Medium.ttf",
        L"fonts\\JetBrainsMono\\JetBrainsMono-SemiBold.ttf",
        L"fonts\\JetBrainsMono\\JetBrainsMono-Bold.ttf",
        L"fonts\\JetBrainsMono\\JetBrainsMono-ExtraBold.ttf",
        L"fonts\\Manrope\\Manrope-Regular.ttf",
        L"fonts\\Manrope\\Manrope-Medium.ttf",
        L"fonts\\Manrope\\Manrope-SemiBold.ttf",
        L"fonts\\Manrope\\Manrope-Bold.ttf",
        L"fonts\\Manrope\\Manrope-ExtraBold.ttf",
        L"fonts\\IBMPlexMono\\IBMPlexMono-Regular.ttf",
        L"fonts\\IBMPlexMono\\IBMPlexMono-Medium.ttf",
        L"fonts\\IBMPlexMono\\IBMPlexMono-SemiBold.ttf",
        L"fonts\\SpaceGrotesk\\SpaceGrotesk-Regular.ttf",
        L"fonts\\SpaceGrotesk\\SpaceGrotesk-Medium.ttf",
        L"fonts\\SpaceGrotesk\\SpaceGrotesk-SemiBold.ttf",
        L"fonts\\SpaceGrotesk\\SpaceGrotesk-Bold.ttf",
    };
    std::wstring dir = exeDir();
    for (const wchar_t* f : files) {
        std::wstring path = dir + f;
        AddFontResourceExW(path.c_str(), FR_PRIVATE, nullptr);
    }
}

static void removeBundledFonts() {
    RemoveFontResourceExW((exeDir() + L"fonts\\").c_str(), FR_PRIVATE, nullptr);
}

static void normalizeFracs() {
    const double minF = 0.08;
    if (g.fracDec < minF) g.fracDec = minF;
    if (g.fracHex < minF) g.fracHex = minF;
    if (g.fracHist < minF) g.fracHist = minF;
    double sum = g.fracDec + g.fracHex + g.fracHist;
    if (sum <= 0.0) {
        g.fracDec = g.fracHex = 0.36;
        g.fracHist = 0.28;
        return;
    }
    g.fracDec /= sum;
    g.fracHex /= sum;
    g.fracHist /= sum;
}

static void layoutControls(int cx, int cy) {
    g.clientW = cx;
    g.clientH = cy;

    const int m = 14;
    const int gap = 8;
    const int rowH = 26;
    const int lblH = 16;
    const int btnH = 32;
    const int statusH = 22;
    const int headerH = 52;

    int contentW = cx - 2 * m;
    if (contentW < 200) contentW = 200;

    // --- header: cim/alcim balra, Nyelv + Megjelenes jobbra ---
    int y = m;
    int langW = 108, themeW = 130, hdrGap = 10;
    int titleW = contentW - langW - themeW - 2 * hdrGap;
    if (titleW < 120) titleW = 120;
    auto place = [](HWND wnd, int x, int yy, int w, int h) {
        if (wnd) MoveWindow(wnd, x, yy, w, h, TRUE);
    };
    place(g.hLblAppTitle, m, y, titleW, 24);
    place(g.hLblAppSubtitle, m, y + 24, titleW, 18);
    place(g.hLblLang, m + contentW - langW - hdrGap - themeW, y, langW, lblH);
    place(g.hComboLang, m + contentW - langW - hdrGap - themeW, y + lblH + 2, langW, rowH);
    place(g.hLblTheme, m + contentW - themeW, y, themeW, lblH);
    place(g.hComboTheme, m + contentW - themeW, y + lblH + 2, themeW, rowH);
    y += headerH;

    // --- Bal / Muvelet / Jobb / Pontossag ---
    int opW = 60;
    int precW = 84;
    int midGap = gap;
    int avail = contentW - opW - precW - 2 * midGap;
    int leftW = avail * 55 / 100;
    int rightW = avail - leftW;

    place(GetDlgItem(g.hwnd, ID_LBL_LEFT), m, y, leftW, lblH);
    place(GetDlgItem(g.hwnd, ID_LBL_OP), m + leftW + midGap, y, opW, lblH);
    place(GetDlgItem(g.hwnd, ID_LBL_RIGHT), m + leftW + midGap + opW + midGap, y, rightW, lblH);
    place(GetDlgItem(g.hwnd, ID_LBL_PREC), m + leftW + midGap + opW + midGap + rightW + midGap, y, precW, lblH);
    y += lblH + 2;

    place(g.hLeft, m, y, leftW, rowH);
    place(g.hOp, m + leftW + midGap, y, opW, rowH + 120);
    place(g.hRight, m + leftW + midGap + opW + midGap, y, rightW, rowH);
    place(g.hPrec, m + leftW + midGap + opW + midGap + rightW + midGap, y, precW, rowH);
    y += rowH + gap;

    place(GetDlgItem(g.hwnd, ID_LBL_EXPR), m, y, contentW, lblH);
    y += lblH + 2;

    int btnW = 112;
    int exprW = contentW - 2 * btnW - 2 * gap;
    if (exprW < 120) exprW = 120;
    place(g.hExpr, m, y, exprW, rowH);
    place(g.hBtnCalc, m + exprW + gap, y - 2, btnW, btnH);
    place(g.hBtnClear, m + exprW + gap + btnW + gap, y - 2, btnW, btnH);
    y += btnH + gap + 6;

    // Also harom panel + 2 huzhato splitter
    int bottom = cy - m - statusH - gap;
    int remain = bottom - y;
    if (remain < 160) remain = 160;

    const int chrome = 3 * (lblH + 2) + 2 * SPLIT_THICK;
    int panelSpace = remain - chrome;
    if (panelSpace < 3 * PANEL_MIN_H) panelSpace = 3 * PANEL_MIN_H;

    normalizeFracs();

    int decH = (int)(panelSpace * g.fracDec + 0.5);
    int hexH = (int)(panelSpace * g.fracHex + 0.5);
    int histH = panelSpace - decH - hexH;
    if (decH < PANEL_MIN_H) decH = PANEL_MIN_H;
    if (hexH < PANEL_MIN_H) hexH = PANEL_MIN_H;
    if (histH < PANEL_MIN_H) histH = PANEL_MIN_H;

    int labelDecY = y;
    place(GetDlgItem(g.hwnd, ID_LBL_DEC), m, y, contentW, lblH);
    y += lblH + 2;
    g.yDecTop = y;
    g.hDecPx = decH;
    place(g.hDec, m, y, contentW, decH);
    g.rcDecCard = { m - CARD_PAD, labelDecY - CARD_PAD, m + contentW + CARD_PAD, y + decH + CARD_PAD };
    y += decH;

    place(g.hSplitDecHex, m, y, contentW, SPLIT_THICK);
    y += SPLIT_THICK;

    int labelHexY = y;
    place(GetDlgItem(g.hwnd, ID_LBL_HEX), m, y, contentW, lblH);
    y += lblH + 2;
    g.yHexTop = y;
    g.hHexPx = hexH;
    place(g.hHex, m, y, contentW, hexH);
    g.rcHexCard = { m - CARD_PAD, labelHexY - CARD_PAD, m + contentW + CARD_PAD, y + hexH + CARD_PAD };
    y += hexH;

    place(g.hSplitHexHist, m, y, contentW, SPLIT_THICK);
    y += SPLIT_THICK;

    int labelHistY = y;
    place(GetDlgItem(g.hwnd, ID_LBL_HIST), m, y, contentW, lblH);
    y += lblH + 2;
    g.yHistTop = y;
    g.hHistPx = histH;
    place(g.hHist, m, y, contentW, histH);
    g.rcHistCard = { m - CARD_PAD, labelHistY - CARD_PAD, m + contentW + CARD_PAD, y + histH + CARD_PAD };

    place(g.hStatus, m, cy - m - statusH, contentW, statusH);

    if (g.hwnd) InvalidateRect(g.hwnd, nullptr, FALSE);
}

static void applySplitDrag(int splitId, int clientY) {
    int total = g.hDecPx + g.hHexPx + g.hHistPx;
    if (total <= 0) return;

    if (splitId == 1) {
        int newDec = clientY - g.yDecTop;
        int pair = g.hDecPx + g.hHexPx;
        if (newDec < PANEL_MIN_H) newDec = PANEL_MIN_H;
        if (newDec > pair - PANEL_MIN_H) newDec = pair - PANEL_MIN_H;
        int newHex = pair - newDec;
        g.fracDec = (double)newDec / total;
        g.fracHex = (double)newHex / total;
        g.fracHist = (double)g.hHistPx / total;
    } else if (splitId == 2) {
        int newHexBottom = clientY;
        int newHex = newHexBottom - g.yHexTop;
        int pair = g.hHexPx + g.hHistPx;
        if (newHex < PANEL_MIN_H) newHex = PANEL_MIN_H;
        if (newHex > pair - PANEL_MIN_H) newHex = pair - PANEL_MIN_H;
        int newHist = pair - newHex;
        g.fracDec = (double)g.hDecPx / total;
        g.fracHex = (double)newHex / total;
        g.fracHist = (double)newHist / total;
    }
    normalizeFracs();
    layoutControls(g.clientW, g.clientH);
}

static LRESULT CALLBACK SplitterProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SETCURSOR:
        SetCursor(LoadCursor(nullptr, IDC_SIZENS));
        return TRUE;
    case WM_ERASEBKGND:
        return 1;
    case WM_LBUTTONDOWN: {
        SetCapture(hwnd);
        int id = GetDlgCtrlID(hwnd);
        g.dragSplit = (id == ID_SPLIT_DEC_HEX) ? 1 : 2;
        return 0;
    }
    case WM_MOUSEMOVE:
        if (g.dragSplit && (wParam & MK_LBUTTON)) {
            POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            MapWindowPoints(hwnd, g.hwnd, &pt, 1);
            applySplitDrag(g.dragSplit, pt.y);
        }
        return 0;
    case WM_LBUTTONUP:
        if (g.dragSplit) {
            ReleaseCapture();
            g.dragSplit = 0;
        }
        return 0;
    case WM_CAPTURECHANGED:
        g.dragSplit = 0;
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        const ThemePalette& p = paletteFor(g_theme);
        HBRUSH b = CreateSolidBrush(p.winBg);
        FillRect(hdc, &rc, b);
        DeleteObject(b);
        int mid = (rc.top + rc.bottom) / 2;
        HPEN pen = CreatePen(PS_SOLID, 1, p.panelBorder);
        HGDIOBJ old = SelectObject(hdc, pen);
        MoveToEx(hdc, rc.left + 8, mid, nullptr);
        LineTo(hdc, rc.right - 8, mid);
        SelectObject(hdc, old);
        DeleteObject(pen);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void registerSplitterClass(HINSTANCE hInst) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = SplitterProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_SIZENS);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"BigCalcSplitter";
    RegisterClassExW(&wc);
}

// --- ListView (Elozmenyek) fejlecenek egyeni szinezese subclass-szal ---
static LRESULT CALLBACK HeaderSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_ERASEBKGND)
        return 1;
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        const ThemePalette& p = paletteFor(g_theme);
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        HBRUSH bg = CreateSolidBrush(p.tableHeaderBg);
        FillRect(hdc, &rcClient, bg);
        DeleteObject(bg);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, p.tableHeaderText);
        HFONT font = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0);
        HGDIOBJ oldFont = font ? SelectObject(hdc, font) : nullptr;

        int count = Header_GetItemCount(hwnd);
        HPEN pen = CreatePen(PS_SOLID, 1, p.panelBorder);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        for (int i = 0; i < count; ++i) {
            RECT rcItem;
            Header_GetItemRect(hwnd, i, &rcItem);
            wchar_t buf[256] = {};
            HDITEMW hdi{};
            hdi.mask = HDI_TEXT;
            hdi.pszText = buf;
            hdi.cchTextMax = 255;
            Header_GetItem(hwnd, i, &hdi);
            RECT rcText = rcItem;
            rcText.left += 6;
            DrawTextW(hdc, buf, -1, &rcText, DT_VCENTER | DT_SINGLELINE | DT_LEFT);
            MoveToEx(hdc, rcItem.right - 1, rcItem.top + 4, nullptr);
            LineTo(hdc, rcItem.right - 1, rcItem.bottom - 4);
        }
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        if (oldFont) SelectObject(hdc, oldFont);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return CallWindowProcW(g_origHeaderProc, hwnd, msg, wParam, lParam);
}

static void subclassHistHeader() {
    HWND hHeader = ListView_GetHeader(g.hHist);
    if (!hHeader || g_origHeaderProc) return;
    g_origHeaderProc = (WNDPROC)SetWindowLongPtrW(hHeader, GWLP_WNDPROC, (LONG_PTR)HeaderSubclassProc);
}

static char selectedOp() {
    int sel = (int)SendMessageW(g.hOp, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) return '+';
    const char ops[] = {'+', '-', '*', '/'};
    if (sel < 0 || sel > 3) return '+';
    return ops[sel];
}

static int selectedPrecision() {
    std::string s = narrow(getText(g.hPrec));
    if (s.empty()) return 3;
    try {
        return std::stoi(s);
    } catch (...) {
        return 3;
    }
}

static void syncExprFromParts() {
    std::wstring left = getText(g.hLeft);
    std::wstring right = getText(g.hRight);
    wchar_t op = static_cast<wchar_t>(selectedOp());
    std::wstring expr = left + L" " + op + L" " + right;
    setText(g.hExpr, expr);
}

static void runCalculate(bool fromParts, bool showPopup) {
    if (!g.ready) return;

    std::string expr;
    if (fromParts) {
        syncExprFromParts();
        expr = narrow(getText(g.hExpr));
    } else {
        expr = narrow(getText(g.hExpr));
        std::string left, right;
        char op = 0;
        if (splitExpression(expr, left, op, right)) {
            setText(g.hLeft, widen(left));
            setText(g.hRight, widen(right));
            const char ops[] = {'+', '-', '*', '/'};
            for (int i = 0; i < 4; ++i) {
                if (ops[i] == op) {
                    SendMessageW(g.hOp, CB_SETCURSEL, i, 0);
                    break;
                }
            }
        }
    }

    int prec = selectedPrecision();
    UnifiedResult r = evaluateSmartExpression(expr, prec);

    if (!r.ok) {
        setText(g.hDec, L"");
        setText(g.hHex, L"");
        setStatus(widen(T(Msg::StatusErrorPrefix)) + widen(r.error));
        if (showPopup)
            MessageBoxW(g.hwnd, widen(r.error).c_str(), widen(T(Msg::MsgBoxTitle)).c_str(), MB_ICONWARNING);
        return;
    }

    if (r.isPointResult) {
        setText(g.hDec, widen("[" + r.mode + "] " + r.decimal + "\r\nX=" + r.pointX + "\r\nY=" + r.pointY));
        setText(g.hHex, widen(r.hex));
    } else {
        setText(g.hDec, widen("[" + r.mode + "] " + r.decimal));
        setText(g.hHex, widen(r.hex));
    }

    BitcoinKeyInfo btc;
    if (r.isPointResult && !r.pointCompressed.empty()) {
        btc = deriveFromCompressedPub(r.pointCompressed);
    } else if (!r.intDecimal.empty()) {
        btc = deriveBitcoinKeys(r.intDecimal);
    }

    std::wstring wif = btc.ok ? widen(btc.wif) : L"-";
    std::wstring pub = btc.ok ? widen(btc.pubCompressed) : (r.isPointResult ? widen(r.pointCompressed) : L"-");
    std::wstring addr = btc.ok ? widen(btc.address) : L"-";
    std::wstring resultCol = r.isPointResult ? widen(r.pointCompressed) : widen(r.decimal);
    if (!btc.ok && !btc.error.empty())
        resultCol += L"  [" + widen(btc.error) + L"]";

    LVITEMW item{};
    item.mask = LVIF_TEXT;
    item.iItem = 0;
    item.iSubItem = 0;
    std::wstring exprW = widen(r.expression);
    item.pszText = exprW.data();
    int row = (int)SendMessageW(g.hHist, LVM_INSERTITEMW, 0, (LPARAM)&item);
    if (row >= 0) {
        ListView_SetItemText(g.hHist, row, 1, resultCol.data());
        ListView_SetItemText(g.hHist, row, 2, wif.data());
        ListView_SetItemText(g.hHist, row, 3, pub.data());
        ListView_SetItemText(g.hHist, row, 4, addr.data());
    }

    if (!r.syncMessage.empty())
        setStatus(widen(r.syncMessage));
    else
        setStatus(widen(T(Msg::StatusModePrefix)) + widen(r.mode) +
                  (btc.ok ? (widen(T(Msg::StatusAddressSuffix)) + addr) : L""));
}

static std::wstring historyColName(int col) {
    switch (col) {
    case 0: return widen(T(Msg::ColExpr));
    case 1: return widen(T(Msg::ColResult));
    case 2: return widen(T(Msg::ColWif));
    case 3: return widen(T(Msg::ColPub));
    case 4: return widen(T(Msg::ColAddress));
    default: return widen(T(Msg::ColCell));
    }
}

static bool copyTextToClipboard(const std::wstring& text) {
    if (!OpenClipboard(g.hwnd)) return false;
    EmptyClipboard();
    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) {
        CloseClipboard();
        return false;
    }
    void* p = GlobalLock(hMem);
    if (!p) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }
    memcpy(p, text.c_str(), bytes);
    GlobalUnlock(hMem);
    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
    return true;
}

static std::wstring historyCellText(int row, int col) {
    if (row < 0 || col < 0) return L"";
    wchar_t buf[4096] = {};
    ListView_GetItemText(g.hHist, row, col, buf, 4096);
    return buf;
}

static bool historyHitTest(const POINT& ptClient, int& row, int& col) {
    LVHITTESTINFO hit{};
    hit.pt = ptClient;
    int idx = ListView_SubItemHitTest(g.hHist, &hit);
    if (idx < 0 || !(hit.flags & LVHT_ONITEM)) return false;
    row = hit.iItem;
    col = hit.iSubItem;
    return true;
}

static void selectHistoryCell(int row, int col) {
    g.histRow = row;
    g.histCol = col;
    if (row >= 0) {
        ListView_SetItemState(g.hHist, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_SetItemState(g.hHist, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(g.hHist, row, FALSE);
    }
}

static void copyHistoryCell(int row, int col) {
    std::wstring text = historyCellText(row, col);
    if (text.empty() || text == L"-") {
        setStatus(widen(T(Msg::StatusNoCopyableData)));
        return;
    }
    if (copyTextToClipboard(text))
        setStatus(widen(T(Msg::StatusCopiedPrefix)) + historyColName(col) + widen(T(Msg::StatusCopiedSuffix)) + text);
    else
        setStatus(widen(T(Msg::StatusCopyFailed)));
}

static void copyHistoryRow(int row) {
    if (row < 0) return;
    std::wstring line;
    for (int c = 0; c < 5; ++c) {
        if (c) line += L'\t';
        line += historyCellText(row, c);
    }
    if (copyTextToClipboard(line))
        setStatus(widen(T(Msg::StatusFullRowCopied)));
    else
        setStatus(widen(T(Msg::StatusCopyFailed)));
}

static void showHistoryContextMenu(HWND hwnd, POINT ptScreen) {
    POINT ptClient = ptScreen;
    ScreenToClient(g.hHist, &ptClient);
    int row = g.histRow, col = g.histCol;
    if (historyHitTest(ptClient, row, col))
        selectHistoryCell(row, col);
    if (g.histRow < 0) return;

    HMENU menu = CreatePopupMenu();
    std::wstring cellLabel = widen(T(Msg::CtxCopyCellPrefix)) + historyColName(g.histCol);
    AppendMenuW(menu, MF_STRING, ID_CTX_COPY_CELL, cellLabel.c_str());
    AppendMenuW(menu, MF_STRING, ID_CTX_COPY_ROW, widen(T(Msg::CtxCopyRow)).c_str());
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}

static void initHistoryColumns() {
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    auto addCol = [&](int idx, const std::wstring& title, int width) {
        col.iSubItem = idx;
        col.cx = width;
        col.pszText = const_cast<wchar_t*>(title.c_str());
        SendMessageW(g.hHist, LVM_INSERTCOLUMNW, idx, (LPARAM)&col);
    };
    addCol(0, widen(T(Msg::ColExpr)), 190);
    addCol(1, widen(T(Msg::ColResult)), 170);
    addCol(2, widen(T(Msg::ColWif)), 270);
    addCol(3, widen(T(Msg::ColPub)), 270);
    addCol(4, widen(T(Msg::ColAddress)), 230);

    ListView_SetExtendedListViewStyle(g.hHist,
        LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
}

// Nyelvvaltaskor: oszlopfejlecek ujraforditasa (a ListView nem ad direkt "SetColumnText"-et,
// ezert LVM_SETCOLUMNW-t hasznalunk a mar letezo oszlopokon).
static void relabelHistoryColumns() {
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT;
    auto setCol = [&](int idx, const std::wstring& title) {
        col.pszText = const_cast<wchar_t*>(title.c_str());
        SendMessageW(g.hHist, LVM_SETCOLUMNW, idx, (LPARAM)&col);
    };
    setCol(0, widen(T(Msg::ColExpr)));
    setCol(1, widen(T(Msg::ColResult)));
    setCol(2, widen(T(Msg::ColWif)));
    setCol(3, widen(T(Msg::ColPub)));
    setCol(4, widen(T(Msg::ColAddress)));
}

// Osszes GUI-chrome szoveg ujraforditasa az aktualis g_lang szerint.
static void applyTranslations() {
    if (g.hwnd) SetWindowTextW(g.hwnd, widen(T(Msg::WinTitle)).c_str());
    setText(g.hLblAppTitle, L"BigCalc");
    setText(g.hLblAppSubtitle, widen(T(Msg::AppSubtitle)));
    setText(GetDlgItem(g.hwnd, ID_LBL_LEFT), widen(T(Msg::LblLeft)));
    setText(GetDlgItem(g.hwnd, ID_LBL_OP), widen(T(Msg::LblOp)));
    setText(GetDlgItem(g.hwnd, ID_LBL_RIGHT), widen(T(Msg::LblRight)));
    setText(GetDlgItem(g.hwnd, ID_LBL_PREC), widen(T(Msg::LblPrec)));
    setText(GetDlgItem(g.hwnd, ID_LBL_EXPR), widen(T(Msg::LblExpr)));
    setText(GetDlgItem(g.hwnd, ID_LBL_DEC), widen(T(Msg::LblDec)));
    setText(GetDlgItem(g.hwnd, ID_LBL_HEX), widen(T(Msg::LblHex)));
    setText(GetDlgItem(g.hwnd, ID_LBL_HIST), widen(T(Msg::LblHist)));
    setText(GetDlgItem(g.hwnd, ID_LBL_LANG), widen(T(Msg::LangLabel)));
    setText(g.hLblTheme, widen(T(Msg::ThemeLabel)));
    setText(g.hBtnCalc, widen(T(Msg::BtnCalc)));
    setText(g.hBtnClear, widen(T(Msg::BtnClear)));

    // Tema-combo felirat ujraforditasa, kivalasztott index megorzesevel
    int sel = (int)SendMessageW(g.hComboTheme, CB_GETCURSEL, 0, 0);
    SendMessageW(g.hComboTheme, CB_RESETCONTENT, 0, 0);
    SendMessageW(g.hComboTheme, CB_ADDSTRING, 0, (LPARAM)widen(T(Msg::ThemeTerminal)).c_str());
    SendMessageW(g.hComboTheme, CB_ADDSTRING, 0, (LPARAM)widen(T(Msg::ThemeLight)).c_str());
    SendMessageW(g.hComboTheme, CB_ADDSTRING, 0, (LPARAM)widen(T(Msg::ThemeFintech)).c_str());
    SendMessageW(g.hComboTheme, CB_SETCURSEL, sel < 0 ? (WPARAM)g_theme : (WPARAM)sel, 0);

    if (g.hHist) relabelHistoryColumns();
    setStatus(widen(T(Msg::StatusReady)));
}

static HWND makeLabel(HWND parent, int id, const wchar_t* text) {
    return CreateWindowExW(0, L"STATIC", text,
        WS_CHILD | WS_VISIBLE,
        0, 0, 10, 10, parent, (HMENU)(intptr_t)id, GetModuleHandleW(nullptr), nullptr);
}

static HWND makeEdit(HWND parent, int id, bool multiline, bool readOnly) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL;
    if (multiline) style |= ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL;
    if (readOnly) style |= ES_READONLY;
    return CreateWindowExW(0, L"EDIT", L"",
        style, 0, 0, 10, 10, parent, (HMENU)(intptr_t)id, GetModuleHandleW(nullptr), nullptr);
}

// --- Betutipusok (ujra)letrehozasa az aktualis tema szerint ---
static void rebuildFonts() {
    const ThemePalette& p = paletteFor(g_theme);
    if (g.hFontTitle) DeleteObject(g.hFontTitle);
    if (g.hFontUi) DeleteObject(g.hFontUi);
    if (g.hFontUiSemibold) DeleteObject(g.hFontUiSemibold);
    if (g.hFontMono) DeleteObject(g.hFontMono);

    g.hFontTitle = CreateFontW(-22, 0, 0, 0, FW_EXTRABOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, p.uiFontTitle);
    g.hFontUi = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, p.uiFont);
    g.hFontUiSemibold = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, p.uiFont);
    g.hFontMono = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, p.monoFont);
}

static void applyFonts() {
    SendMessageW(g.hLblAppTitle, WM_SETFONT, (WPARAM)g.hFontTitle, TRUE);

    HWND uiWidgets[] = {
        g.hLblAppSubtitle, GetDlgItem(g.hwnd, ID_LBL_LEFT), GetDlgItem(g.hwnd, ID_LBL_OP),
        GetDlgItem(g.hwnd, ID_LBL_RIGHT), GetDlgItem(g.hwnd, ID_LBL_PREC), GetDlgItem(g.hwnd, ID_LBL_EXPR),
        g.hLblLang, g.hComboLang, g.hLblTheme, g.hComboTheme, g.hOp, g.hStatus
    };
    for (HWND h : uiWidgets)
        if (h) SendMessageW(h, WM_SETFONT, (WPARAM)g.hFontUi, TRUE);

    HWND semiboldWidgets[] = {
        GetDlgItem(g.hwnd, ID_LBL_DEC), GetDlgItem(g.hwnd, ID_LBL_HEX), GetDlgItem(g.hwnd, ID_LBL_HIST),
        g.hBtnCalc, g.hBtnClear
    };
    for (HWND h : semiboldWidgets)
        if (h) SendMessageW(h, WM_SETFONT, (WPARAM)g.hFontUiSemibold, TRUE);

    HWND monoWidgets[] = { g.hDec, g.hHex, g.hHist, g.hExpr, g.hLeft, g.hRight, g.hPrec };
    for (HWND h : monoWidgets)
        if (h) SendMessageW(h, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);
}

static void applyListViewColors() {
    const ThemePalette& p = paletteFor(g_theme);
    ListView_SetBkColor(g.hHist, p.tableRowBg);
    ListView_SetTextColor(g.hHist, p.text);
    ListView_SetTextBkColor(g.hHist, p.tableRowBg);
    HWND hHeader = ListView_GetHeader(g.hHist);
    if (hHeader) {
        SendMessageW(hHeader, WM_SETFONT, (WPARAM)g.hFontUiSemibold, TRUE);
        InvalidateRect(hHeader, nullptr, TRUE);
    }
    InvalidateRect(g.hHist, nullptr, TRUE);
}

// Teljes megjeleneskor-valto: betutipusok, ecsetek, ListView szinek, ujrarajzolas.
static void applyTheme(Theme t) {
    g_theme = t;
    rebuildFonts();
    applyFonts();

    const ThemePalette& p = paletteFor(t);
    if (g.hBgBrush) DeleteObject(g.hBgBrush);
    if (g.hInputBrush) DeleteObject(g.hInputBrush);
    g.hBgBrush = CreateSolidBrush(p.winBg);
    g.hInputBrush = CreateSolidBrush(p.inputBg);

    applyListViewColors();

    int sel = (int)SendMessageW(g.hComboTheme, CB_GETCURSEL, 0, 0);
    if (sel != (int)t) SendMessageW(g.hComboTheme, CB_SETCURSEL, (WPARAM)t, 0);

    if (g.hwnd) {
        InvalidateRect(g.hwnd, nullptr, TRUE);
        UpdateWindow(g.hwnd);
    }
}

// --- Ownerdraw rajzolo segedfuggvenyek ---

static void drawThemedButton(LPDRAWITEMSTRUCT dis, bool primary) {
    const ThemePalette& p = paletteFor(g_theme);
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;

    COLORREF bgColor = primary ? p.accent : p.panelBg;
    COLORREF borderColor = primary ? p.accent : p.panelBorder;
    COLORREF textColor = primary ? p.accentText : p.btnSecondaryText;

    HBRUSH bg = CreateSolidBrush(bgColor);
    FillRect(dis->hDC, &dis->rcItem, bg);
    DeleteObject(bg);

    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HGDIOBJ oldPen = SelectObject(dis->hDC, pen);
    HGDIOBJ oldBrush = SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
    Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom);
    SelectObject(dis->hDC, oldBrush);
    SelectObject(dis->hDC, oldPen);
    DeleteObject(pen);

    wchar_t text[128];
    GetWindowTextW(dis->hwndItem, text, 128);
    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, textColor);
    HFONT font = (HFONT)SendMessageW(dis->hwndItem, WM_GETFONT, 0, 0);
    HGDIOBJ oldFont = font ? SelectObject(dis->hDC, font) : nullptr;
    RECT rc = dis->rcItem;
    if (pressed) OffsetRect(&rc, 1, 1);
    DrawTextW(dis->hDC, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (oldFont) SelectObject(dis->hDC, oldFont);

    if (dis->itemState & ODS_FOCUS) {
        RECT rcFocus = dis->rcItem;
        InflateRect(&rcFocus, -3, -3);
        DrawFocusRect(dis->hDC, &rcFocus);
    }
}

static void drawThemedComboItem(LPDRAWITEMSTRUCT dis) {
    const ThemePalette& p = paletteFor(g_theme);
    bool selected = (dis->itemState & ODS_SELECTED) != 0;

    COLORREF bg = selected ? p.accent : p.inputBg;
    COLORREF fg = selected ? p.accentText : p.text;
    HBRUSH bgBrush = CreateSolidBrush(bg);
    FillRect(dis->hDC, &dis->rcItem, bgBrush);
    DeleteObject(bgBrush);

    wchar_t buf[256] = {};
    if ((int)dis->itemID >= 0)
        SendMessageW(dis->hwndItem, CB_GETLBTEXT, dis->itemID, (LPARAM)buf);

    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, fg);
    HFONT font = (HFONT)SendMessageW(dis->hwndItem, WM_GETFONT, 0, 0);
    HGDIOBJ oldFont = font ? SelectObject(dis->hDC, font) : nullptr;
    RECT rc = dis->rcItem;
    rc.left += 6;
    DrawTextW(dis->hDC, buf, -1, &rc, DT_VCENTER | DT_SINGLELINE | DT_LEFT);
    if (oldFont) SelectObject(dis->hDC, oldFont);

    if (dis->itemState & ODS_FOCUS)
        DrawFocusRect(dis->hDC, &dis->rcItem);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        g.hwnd = hwnd;
        INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icc);

        rebuildFonts();

        g.hLblAppTitle = makeLabel(hwnd, ID_LBL_APPTITLE, L"BigCalc");
        g.hLblAppSubtitle = makeLabel(hwnd, ID_LBL_APPSUBTITLE, L"");

        makeLabel(hwnd, ID_LBL_LEFT, widen(T(Msg::LblLeft)).c_str());
        makeLabel(hwnd, ID_LBL_OP, widen(T(Msg::LblOp)).c_str());
        makeLabel(hwnd, ID_LBL_RIGHT, widen(T(Msg::LblRight)).c_str());
        makeLabel(hwnd, ID_LBL_PREC, widen(T(Msg::LblPrec)).c_str());
        makeLabel(hwnd, ID_LBL_EXPR, widen(T(Msg::LblExpr)).c_str());
        makeLabel(hwnd, ID_LBL_DEC, widen(T(Msg::LblDec)).c_str());
        makeLabel(hwnd, ID_LBL_HEX, widen(T(Msg::LblHex)).c_str());
        makeLabel(hwnd, ID_LBL_HIST, widen(T(Msg::LblHist)).c_str());
        g.hLblLang = makeLabel(hwnd, ID_LBL_LANG, widen(T(Msg::LangLabel)).c_str());
        g.hLblTheme = makeLabel(hwnd, ID_LBL_THEME, widen(T(Msg::ThemeLabel)).c_str());

        g.hLeft = makeEdit(hwnd, ID_EDIT_LEFT, false, false);
        g.hRight = makeEdit(hwnd, ID_EDIT_RIGHT, false, false);
        g.hPrec = makeEdit(hwnd, ID_EDIT_PREC, false, false);
        g.hExpr = makeEdit(hwnd, ID_EDIT_EXPR, false, false);
        g.hDec = makeEdit(hwnd, ID_EDIT_DEC, true, true);
        g.hHex = makeEdit(hwnd, ID_EDIT_HEX, true, true);

        g.hOp = CreateWindowExW(0, L"COMBOBOX", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED,
            0, 0, 10, 10, hwnd, (HMENU)ID_COMBO_OP, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(g.hOp, CB_ADDSTRING, 0, (LPARAM)L"+");
        SendMessageW(g.hOp, CB_ADDSTRING, 0, (LPARAM)L"-");
        SendMessageW(g.hOp, CB_ADDSTRING, 0, (LPARAM)L"*");
        SendMessageW(g.hOp, CB_ADDSTRING, 0, (LPARAM)L"/");
        SendMessageW(g.hOp, CB_SETCURSEL, 2, 0); // *
        SendMessageW(g.hOp, CB_SETITEMHEIGHT, (WPARAM)-1, 22);
        SendMessageW(g.hOp, CB_SETITEMHEIGHT, 0, 22);

        g.hComboLang = CreateWindowExW(0, L"COMBOBOX", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED,
            0, 0, 10, 10, hwnd, (HMENU)ID_COMBO_LANG, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(g.hComboLang, CB_ADDSTRING, 0, (LPARAM)L"Magyar");
        SendMessageW(g.hComboLang, CB_ADDSTRING, 0, (LPARAM)L"English");
        SendMessageW(g.hComboLang, CB_ADDSTRING, 0, (LPARAM)L"Deutsch");
        SendMessageW(g.hComboLang, CB_SETCURSEL, (WPARAM)g_lang, 0);
        SendMessageW(g.hComboLang, CB_SETITEMHEIGHT, (WPARAM)-1, 22);
        SendMessageW(g.hComboLang, CB_SETITEMHEIGHT, 0, 22);

        g.hComboTheme = CreateWindowExW(0, L"COMBOBOX", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED,
            0, 0, 10, 10, hwnd, (HMENU)ID_COMBO_THEME, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(g.hComboTheme, CB_ADDSTRING, 0, (LPARAM)L"Terminal");
        SendMessageW(g.hComboTheme, CB_ADDSTRING, 0, (LPARAM)L"Light");
        SendMessageW(g.hComboTheme, CB_ADDSTRING, 0, (LPARAM)L"Fintech");
        SendMessageW(g.hComboTheme, CB_SETCURSEL, (WPARAM)g_theme, 0);
        SendMessageW(g.hComboTheme, CB_SETITEMHEIGHT, (WPARAM)-1, 22);
        SendMessageW(g.hComboTheme, CB_SETITEMHEIGHT, 0, 22);

        g.hBtnCalc = CreateWindowExW(0, L"BUTTON", widen(T(Msg::BtnCalc)).c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
            0, 0, 10, 10, hwnd, (HMENU)ID_BTN_CALC, GetModuleHandleW(nullptr), nullptr);
        g.hBtnClear = CreateWindowExW(0, L"BUTTON", widen(T(Msg::BtnClear)).c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
            0, 0, 10, 10, hwnd, (HMENU)ID_BTN_CLEAR, GetModuleHandleW(nullptr), nullptr);

        g.hHist = CreateWindowExW(0, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL |
            LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            0, 0, 10, 10, hwnd, (HMENU)ID_LIST_HIST, GetModuleHandleW(nullptr), nullptr);
        initHistoryColumns();
        subclassHistHeader();

        g.hSplitDecHex = CreateWindowExW(0, L"BigCalcSplitter", L"",
            WS_CHILD | WS_VISIBLE,
            0, 0, 10, SPLIT_THICK, hwnd, (HMENU)ID_SPLIT_DEC_HEX, GetModuleHandleW(nullptr), nullptr);
        g.hSplitHexHist = CreateWindowExW(0, L"BigCalcSplitter", L"",
            WS_CHILD | WS_VISIBLE,
            0, 0, 10, SPLIT_THICK, hwnd, (HMENU)ID_SPLIT_HEX_HIST, GetModuleHandleW(nullptr), nullptr);

        g.hStatus = CreateWindowExW(0, L"STATIC", widen(T(Msg::StatusReady)).c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0, 0, 10, 10, hwnd, (HMENU)ID_STATUS, GetModuleHandleW(nullptr), nullptr);

        setText(g.hPrec, L"3");
        setText(g.hLeft, L"0xFF");
        setText(g.hRight, L"1.5");
        syncExprFromParts();

        applyTheme(g_theme);
        applyTranslations();
        g.ready = true;
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND hCtrl = (HWND)lParam;
        const ThemePalette& p = paletteFor(g_theme);
        int id = GetDlgCtrlID(hCtrl);

        bool isInputLike = (id == ID_EDIT_LEFT || id == ID_EDIT_RIGHT || id == ID_EDIT_PREC ||
                             id == ID_EDIT_EXPR || id == ID_EDIT_DEC || id == ID_EDIT_HEX);
        if (isInputLike) {
            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, p.inputBg);
            SetTextColor(hdc, p.text);
            return (INT_PTR)g.hInputBrush;
        }

        SetBkMode(hdc, TRANSPARENT);
        COLORREF fg = p.textDim;
        if (id == ID_LBL_APPTITLE) fg = p.accent;
        else if (id == ID_LBL_DEC || id == ID_LBL_HEX || id == ID_LBL_HIST) fg = p.accent;
        else if (id == ID_LBL_APPSUBTITLE || id == ID_STATUS) fg = p.textDim;
        SetTextColor(hdc, fg);
        return (INT_PTR)g.hBgBrush;
    }
    case WM_DRAWITEM: {
        auto* dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
        if (dis->CtlID == ID_BTN_CALC || dis->CtlID == ID_BTN_CLEAR) {
            drawThemedButton(dis, dis->CtlID == ID_BTN_CALC);
            return TRUE;
        }
        if (dis->CtlID == ID_COMBO_OP || dis->CtlID == ID_COMBO_LANG || dis->CtlID == ID_COMBO_THEME) {
            drawThemedComboItem(dis);
            return TRUE;
        }
        return FALSE;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        const ThemePalette& p = paletteFor(g_theme);
        FillRect(hdc, &ps.rcPaint, g.hBgBrush);

        auto drawCard = [&](const RECT& rc) {
            HBRUSH b = CreateSolidBrush(p.panelBg);
            FillRect(hdc, &rc, b);
            DeleteObject(b);
            HPEN pen = CreatePen(PS_SOLID, 1, p.panelBorder);
            HGDIOBJ oldPen = SelectObject(hdc, pen);
            HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);
            DeleteObject(pen);
        };
        drawCard(g.rcDecCard);
        drawCard(g.rcHexCard);
        drawCard(g.rcHistCard);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_SIZE: {
        layoutControls(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
    case WM_GETMINMAXINFO: {
        auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        mmi->ptMinTrackSize.x = 760;
        mmi->ptMinTrackSize.y = 560;
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if (id == ID_BTN_CALC) {
            bool fromParts = (GetFocus() != g.hExpr);
            runCalculate(fromParts, true);
            return 0;
        }
        if (id == ID_BTN_CLEAR) {
            setText(g.hLeft, L"");
            setText(g.hRight, L"");
            setText(g.hExpr, L"");
            setText(g.hDec, L"");
            setText(g.hHex, L"");
            ListView_DeleteAllItems(g.hHist);
            g.histRow = -1;
            g.histCol = 0;
            setStatus(widen(T(Msg::StatusFieldsCleared)));
            return 0;
        }
        if (id == ID_COMBO_LANG && code == CBN_SELCHANGE) {
            int sel = (int)SendMessageW(g.hComboLang, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel <= 2) {
                g_lang = static_cast<Lang>(sel);
                saveLangToIni();
                applyTranslations();
                layoutControls(g.clientW, g.clientH);
            }
            return 0;
        }
        if (id == ID_COMBO_THEME && code == CBN_SELCHANGE) {
            int sel = (int)SendMessageW(g.hComboTheme, CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel <= 2) {
                saveThemeToIni();
                applyTheme(static_cast<Theme>(sel));
            }
            return 0;
        }
        if (id == ID_CTX_COPY_CELL) {
            copyHistoryCell(g.histRow, g.histCol);
            return 0;
        }
        if (id == ID_CTX_COPY_ROW) {
            copyHistoryRow(g.histRow);
            return 0;
        }
        return 0;
    }
    case WM_CONTEXTMENU: {
        if ((HWND)wParam == g.hHist || GetFocus() == g.hHist) {
            POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            if (pt.x == -1 && pt.y == -1) {
                RECT rc{};
                if (g.histRow >= 0)
                    ListView_GetItemRect(g.hHist, g.histRow, &rc, LVIR_BOUNDS);
                else
                    GetClientRect(g.hHist, &rc);
                pt.x = rc.left + 8;
                pt.y = rc.top + 8;
                ClientToScreen(g.hHist, &pt);
            }
            showHistoryContextMenu(hwnd, pt);
            return 0;
        }
        break;
    }
    case WM_NOTIFY: {
        auto* hdr = reinterpret_cast<LPNMHDR>(lParam);
        if (!hdr || hdr->idFrom != ID_LIST_HIST) return 0;

        if (hdr->code == NM_CUSTOMDRAW) {
            auto* cd = reinterpret_cast<LPNMLVCUSTOMDRAW>(lParam);
            const ThemePalette& p = paletteFor(g_theme);
            switch (cd->nmcd.dwDrawStage) {
            case CDDS_PREPAINT:
                return CDRF_NOTIFYITEMDRAW;
            case CDDS_ITEMPREPAINT: {
                bool sel = (ListView_GetItemState(g.hHist, (int)cd->nmcd.dwItemSpec, LVIS_SELECTED) & LVIS_SELECTED) != 0;
                cd->clrText = sel ? p.tableSelText : p.text;
                cd->clrTextBk = sel ? p.tableSelBg : p.tableRowBg;
                return CDRF_DODEFAULT;
            }
            default:
                return CDRF_DODEFAULT;
            }
        }

        if (hdr->code == NM_CLICK || hdr->code == NM_RCLICK || hdr->code == NM_DBLCLK) {
            auto* ia = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
            int row = ia->iItem;
            int col = ia->iSubItem;
            if (row < 0) {
                POINT pt = ia->ptAction;
                historyHitTest(pt, row, col);
            }
            if (row >= 0) {
                selectHistoryCell(row, col);
                if (hdr->code == NM_DBLCLK) {
                    copyHistoryCell(row, col);
                } else if (hdr->code == NM_CLICK) {
                    setStatus(widen(T(Msg::StatusSelectedPrefix)) + historyColName(col) +
                              widen(T(Msg::StatusSelectedSuffix)));
                }
            }
            return 0;
        }
        return 0;
    }
    case WM_DESTROY:
        if (g.hFontTitle) DeleteObject(g.hFontTitle);
        if (g.hFontUi) DeleteObject(g.hFontUi);
        if (g.hFontUiSemibold) DeleteObject(g.hFontUiSemibold);
        if (g.hFontMono) DeleteObject(g.hFontMono);
        if (g.hBgBrush) DeleteObject(g.hBgBrush);
        if (g.hInputBrush) DeleteObject(g.hInputBrush);
        removeBundledFonts();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int nShow) {
    const wchar_t* cls = L"BigCalcGuiWnd";

    loadBundledFonts();
    loadLangFromIni();
    loadThemeFromIni();
    registerSplitterClass(hInst);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = cls;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0, cls, widen(T(Msg::WinTitle)).c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1080, 780,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        // Ctrl+C az elozmeny tabla aktiv cellajara
        if (msg.message == WM_KEYDOWN && msg.wParam == 'C' &&
            (GetKeyState(VK_CONTROL) & 0x8000) &&
            GetFocus() == g.hHist) {
            if (g.histRow < 0)
                g.histRow = ListView_GetNextItem(g.hHist, -1, LVNI_SELECTED);
            if (g.histRow >= 0)
                copyHistoryCell(g.histRow, g.histCol);
            continue;
        }
        if (!IsDialogMessageW(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return (int)msg.wParam;
}
