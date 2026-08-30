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
    ID_CTX_COPY_CELL = 2001,
    ID_CTX_COPY_ROW
};

enum : int {
    SPLIT_THICK = 6,
    PANEL_MIN_H = 40
};

struct AppState {
    HWND hwnd = nullptr;
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
    HFONT hFont = nullptr;
    HFONT hFontMono = nullptr;
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
};

static AppState g;

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

// --- Nyelvi beallitas: bigcalc_lang.ini az exe mellett, egyetlen sor "HU"/"EN"/"DE" ---
static std::wstring exeDirIniPath() {
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::wstring path(buf, n);
    size_t slash = path.find_last_of(L"\\/");
    std::wstring dir = (slash == std::wstring::npos) ? L"" : path.substr(0, slash + 1);
    return dir + langIniName();
}

static void loadLangFromIni() {
    std::ifstream in(exeDirIniPath().c_str());
    if (!in) return;
    std::string code;
    std::getline(in, code);
    while (!code.empty() && (code.back() == '\r' || code.back() == '\n')) code.pop_back();
    g_lang = langFromCode(code);
}

static void saveLangToIni() {
    std::ofstream out(exeDirIniPath().c_str(), std::ios::trunc);
    if (!out) return;
    out << langToCode(g_lang);
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

    const int m = 12;
    const int gap = 8;
    const int rowH = 26;
    const int lblH = 18;
    const int btnH = 32;
    const int statusH = 22;

    int y = m;
    int contentW = cx - 2 * m;
    if (contentW < 200) contentW = 200;

    int opW = 68;
    int precW = 92;
    int langW = 130;
    int midGap = gap;
    int avail = contentW - opW - precW - langW - 4 * midGap;
    int leftW = avail * 45 / 100;
    int rightW = avail - leftW;

    auto place = [](HWND wnd, int x, int yy, int w, int h) {
        if (wnd) MoveWindow(wnd, x, yy, w, h, TRUE);
    };

    place(GetDlgItem(g.hwnd, ID_LBL_LEFT), m, y, leftW, lblH);
    place(GetDlgItem(g.hwnd, ID_LBL_OP), m + leftW + midGap, y, opW, lblH);
    place(GetDlgItem(g.hwnd, ID_LBL_RIGHT), m + leftW + midGap + opW + midGap, y, rightW, lblH);
    place(GetDlgItem(g.hwnd, ID_LBL_PREC), m + leftW + midGap + opW + midGap + rightW + midGap, y, precW, lblH);
    place(g.hLblLang, m + contentW - langW, y, langW, lblH);
    y += lblH + 2;

    place(g.hLeft, m, y, leftW, rowH);
    place(g.hOp, m + leftW + midGap, y, opW, rowH + 120);
    place(g.hRight, m + leftW + midGap + opW + midGap, y, rightW, rowH);
    place(g.hPrec, m + leftW + midGap + opW + midGap + rightW + midGap, y, precW, rowH);
    place(g.hComboLang, m + contentW - langW, y, langW, rowH + 120);
    y += rowH + gap;

    place(GetDlgItem(g.hwnd, ID_LBL_EXPR), m, y, contentW, lblH);
    y += lblH + 2;

    int btnW = 110;
    int exprW = contentW - 2 * btnW - 2 * gap;
    if (exprW < 120) exprW = 120;
    place(g.hExpr, m, y, exprW, rowH);
    place(g.hBtnCalc, m + exprW + gap, y - 2, btnW, btnH);
    place(g.hBtnClear, m + exprW + gap + btnW + gap, y - 2, btnW, btnH);
    y += btnH + gap + 4;

    // Also harom panel + 2 huzzhato splitter
    int bottom = cy - m - statusH - gap;
    int remain = bottom - y;
    if (remain < 160) remain = 160;

    const int chrome = 3 * lblH + 6 + 2 * SPLIT_THICK; // cimkek + splitterek
    int panelSpace = remain - chrome;
    if (panelSpace < 3 * PANEL_MIN_H) panelSpace = 3 * PANEL_MIN_H;

    normalizeFracs();

    int decH = (int)(panelSpace * g.fracDec + 0.5);
    int hexH = (int)(panelSpace * g.fracHex + 0.5);
    int histH = panelSpace - decH - hexH;
    if (decH < PANEL_MIN_H) decH = PANEL_MIN_H;
    if (hexH < PANEL_MIN_H) hexH = PANEL_MIN_H;
    if (histH < PANEL_MIN_H) histH = PANEL_MIN_H;

    place(GetDlgItem(g.hwnd, ID_LBL_DEC), m, y, contentW, lblH);
    y += lblH + 2;
    g.yDecTop = y;
    g.hDecPx = decH;
    place(g.hDec, m, y, contentW, decH);
    y += decH;

    place(g.hSplitDecHex, m, y, contentW, SPLIT_THICK);
    y += SPLIT_THICK;

    place(GetDlgItem(g.hwnd, ID_LBL_HEX), m, y, contentW, lblH);
    y += lblH + 2;
    g.yHexTop = y;
    g.hHexPx = hexH;
    place(g.hHex, m, y, contentW, hexH);
    y += hexH;

    place(g.hSplitHexHist, m, y, contentW, SPLIT_THICK);
    y += SPLIT_THICK;

    place(GetDlgItem(g.hwnd, ID_LBL_HIST), m, y, contentW, lblH);
    y += lblH + 2;
    g.yHistTop = y;
    g.hHistPx = histH;
    place(g.hHist, m, y, contentW, histH);

    place(g.hStatus, m, cy - m - statusH, contentW, statusH);
}

static void applySplitDrag(int splitId, int clientY) {
    int total = g.hDecPx + g.hHexPx + g.hHistPx;
    if (total <= 0) return;

    if (splitId == 1) {
        // Uj dec magassag: a splitter a dec alja
        int newDec = clientY - g.yDecTop;
        int pair = g.hDecPx + g.hHexPx;
        if (newDec < PANEL_MIN_H) newDec = PANEL_MIN_H;
        if (newDec > pair - PANEL_MIN_H) newDec = pair - PANEL_MIN_H;
        int newHex = pair - newDec;
        g.fracDec = (double)newDec / total;
        g.fracHex = (double)newHex / total;
        g.fracHist = (double)g.hHistPx / total;
    } else if (splitId == 2) {
        // Uj hex also hatara = splitter; hist = ami utana marad
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
        FillRect(hdc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
        // Kozepso fogasi vonal
        int mid = (rc.top + rc.bottom) / 2;
        HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
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
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"BigCalcSplitter";
    RegisterClassExW(&wc);
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
    addCol(0, widen(T(Msg::ColExpr)), 220);
    addCol(1, widen(T(Msg::ColResult)), 200);
    addCol(2, widen(T(Msg::ColWif)), 280);
    addCol(3, widen(T(Msg::ColPub)), 280);
    addCol(4, widen(T(Msg::ColAddress)), 220);

    ListView_SetExtendedListViewStyle(g.hHist,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
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
// Nyelvvaltaskor es inditaskor is ez alliltja be a feliratokat.
static void applyTranslations() {
    if (g.hwnd) SetWindowTextW(g.hwnd, widen(T(Msg::WinTitle)).c_str());
    setText(GetDlgItem(g.hwnd, ID_LBL_LEFT), widen(T(Msg::LblLeft)));
    setText(GetDlgItem(g.hwnd, ID_LBL_OP), widen(T(Msg::LblOp)));
    setText(GetDlgItem(g.hwnd, ID_LBL_RIGHT), widen(T(Msg::LblRight)));
    setText(GetDlgItem(g.hwnd, ID_LBL_PREC), widen(T(Msg::LblPrec)));
    setText(GetDlgItem(g.hwnd, ID_LBL_EXPR), widen(T(Msg::LblExpr)));
    setText(GetDlgItem(g.hwnd, ID_LBL_DEC), widen(T(Msg::LblDec)));
    setText(GetDlgItem(g.hwnd, ID_LBL_HEX), widen(T(Msg::LblHex)));
    setText(GetDlgItem(g.hwnd, ID_LBL_HIST), widen(T(Msg::LblHist)));
    setText(GetDlgItem(g.hwnd, ID_LBL_LANG), widen(T(Msg::LangLabel)));
    setText(g.hBtnCalc, widen(T(Msg::BtnCalc)));
    setText(g.hBtnClear, widen(T(Msg::BtnClear)));
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
    return CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        style, 0, 0, 10, 10, parent, (HMENU)(intptr_t)id, GetModuleHandleW(nullptr), nullptr);
}

static void applyFonts(HWND root) {
    EnumChildWindows(root, [](HWND h, LPARAM lp) -> BOOL {
        SendMessageW(h, WM_SETFONT, (WPARAM)lp, TRUE);
        return TRUE;
    }, (LPARAM)g.hFont);

    // Eredmények monospaced a hosszú hex/dec miatt
    SendMessageW(g.hDec, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);
    SendMessageW(g.hHex, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);
    SendMessageW(g.hHist, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);
    SendMessageW(g.hExpr, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);
    SendMessageW(g.hLeft, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);
    SendMessageW(g.hRight, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        g.hwnd = hwnd;
        INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icc);

        g.hFont = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g.hFontMono = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            FIXED_PITCH | FF_MODERN, L"Consolas");

        makeLabel(hwnd, ID_LBL_LEFT, widen(T(Msg::LblLeft)).c_str());
        makeLabel(hwnd, ID_LBL_OP, widen(T(Msg::LblOp)).c_str());
        makeLabel(hwnd, ID_LBL_RIGHT, widen(T(Msg::LblRight)).c_str());
        makeLabel(hwnd, ID_LBL_PREC, widen(T(Msg::LblPrec)).c_str());
        makeLabel(hwnd, ID_LBL_EXPR, widen(T(Msg::LblExpr)).c_str());
        makeLabel(hwnd, ID_LBL_DEC, widen(T(Msg::LblDec)).c_str());
        makeLabel(hwnd, ID_LBL_HEX, widen(T(Msg::LblHex)).c_str());
        makeLabel(hwnd, ID_LBL_HIST, widen(T(Msg::LblHist)).c_str());
        g.hLblLang = makeLabel(hwnd, ID_LBL_LANG, widen(T(Msg::LangLabel)).c_str());

        g.hLeft = makeEdit(hwnd, ID_EDIT_LEFT, false, false);
        g.hRight = makeEdit(hwnd, ID_EDIT_RIGHT, false, false);
        g.hPrec = makeEdit(hwnd, ID_EDIT_PREC, false, false);
        g.hExpr = makeEdit(hwnd, ID_EDIT_EXPR, false, false);
        g.hDec = makeEdit(hwnd, ID_EDIT_DEC, true, true);
        g.hHex = makeEdit(hwnd, ID_EDIT_HEX, true, true);

        g.hOp = CreateWindowExW(0, L"COMBOBOX", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
            0, 0, 10, 10, hwnd, (HMENU)ID_COMBO_OP, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(g.hOp, CB_ADDSTRING, 0, (LPARAM)L"+");
        SendMessageW(g.hOp, CB_ADDSTRING, 0, (LPARAM)L"-");
        SendMessageW(g.hOp, CB_ADDSTRING, 0, (LPARAM)L"*");
        SendMessageW(g.hOp, CB_ADDSTRING, 0, (LPARAM)L"/");
        SendMessageW(g.hOp, CB_SETCURSEL, 2, 0); // *

        g.hComboLang = CreateWindowExW(0, L"COMBOBOX", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
            0, 0, 10, 10, hwnd, (HMENU)ID_COMBO_LANG, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(g.hComboLang, CB_ADDSTRING, 0, (LPARAM)L"Magyar");
        SendMessageW(g.hComboLang, CB_ADDSTRING, 0, (LPARAM)L"English");
        SendMessageW(g.hComboLang, CB_ADDSTRING, 0, (LPARAM)L"Deutsch");
        SendMessageW(g.hComboLang, CB_SETCURSEL, (WPARAM)g_lang, 0);

        g.hBtnCalc = CreateWindowExW(0, L"BUTTON", widen(T(Msg::BtnCalc)).c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            0, 0, 10, 10, hwnd, (HMENU)ID_BTN_CALC, GetModuleHandleW(nullptr), nullptr);
        g.hBtnClear = CreateWindowExW(0, L"BUTTON", widen(T(Msg::BtnClear)).c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            0, 0, 10, 10, hwnd, (HMENU)ID_BTN_CLEAR, GetModuleHandleW(nullptr), nullptr);

        g.hHist = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL |
            LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            0, 0, 10, 10, hwnd, (HMENU)ID_LIST_HIST, GetModuleHandleW(nullptr), nullptr);
        initHistoryColumns();

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

        applyFonts(hwnd);
        g.ready = true;
        return 0;
    }
    case WM_SIZE: {
        layoutControls(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
    case WM_GETMINMAXINFO: {
        auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        mmi->ptMinTrackSize.x = 640;
        mmi->ptMinTrackSize.y = 520;
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if (id == ID_BTN_CALC) {
            // Ha a kifejezés mező aktív, azt használjuk; különben a három részmezőt.
            bool fromParts = (GetFocus() != g.hExpr);
            runCalculate(fromParts, true);
            return 0;
        }
        // Szandekosan NINCS EN_CHANGE automatikus szamitas:
        // a mezok kitoltese inditaskor 4x hibadobozt dobott, es megakasztotta az ablakot.
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
        if (g.hFont) DeleteObject(g.hFont);
        if (g.hFontMono) DeleteObject(g.hFontMono);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int nShow) {
    const wchar_t* cls = L"BigCalcGuiWnd";

    loadLangFromIni();
    registerSplitterClass(hInst);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = cls;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0, cls, widen(T(Msg::WinTitle)).c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 720,
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
