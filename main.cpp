#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QStatusBar>
#include <QMenu>
#include <QClipboard>
#include <QGuiApplication>
#include <QShortcut>
#include <QKeySequence>
#include <QFont>
#include <QFontDatabase>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QGraphicsDropShadowEffect>
#include <QColor>

#include "Calculator.hpp"
#include "Lang.hpp"
#include "Theme.hpp"
#include "ModeEngine.hpp"
#include "crypto/BitcoinCrypto.hpp"

namespace {

QString langIniPath() {
    return QCoreApplication::applicationDirPath() + "/" +
           QString::fromWCharArray(langIniName());
}

void loadLangFromIni() {
    QFile f(langIniPath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    QString code = in.readLine().trimmed();
    g_lang = langFromCode(code.toStdString());
}

void saveLangToIni() {
    QFile f(langIniPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return;
    QTextStream out(&f);
    out << langToCode(g_lang);
}

QString themeIniPath() {
    return QCoreApplication::applicationDirPath() + "/" +
           QString::fromWCharArray(themeIniName());
}

void loadThemeFromIni() {
    QFile f(themeIniPath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    QString code = in.readLine().trimmed();
    g_theme = themeFromCode(code.toStdString());
}

void saveThemeToIni() {
    QFile f(themeIniPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return;
    QTextStream out(&f);
    out << themeToCode(g_theme);
}

// A becsomagolt betutipusok (fonts/) az exe mellett VAGY a projekt gyokereben
// (fejlesztoi build eseten a build/ konyvtarbol futtatva) kereshetok.
QString findFontsDir() {
    const QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/fonts",
        QCoreApplication::applicationDirPath() + "/../fonts",
    };
    for (const auto& c : candidates)
        if (QDir(c).exists()) return c;
    return QString();
}

void loadBundledFonts() {
    const QString dir = findFontsDir();
    if (dir.isEmpty()) return;
    const QStringList files = {
        "JetBrainsMono/JetBrainsMono-Regular.ttf",
        "JetBrainsMono/JetBrainsMono-Medium.ttf",
        "JetBrainsMono/JetBrainsMono-SemiBold.ttf",
        "JetBrainsMono/JetBrainsMono-Bold.ttf",
        "JetBrainsMono/JetBrainsMono-ExtraBold.ttf",
        "Manrope/Manrope-Regular.ttf",
        "Manrope/Manrope-Medium.ttf",
        "Manrope/Manrope-SemiBold.ttf",
        "Manrope/Manrope-Bold.ttf",
        "Manrope/Manrope-ExtraBold.ttf",
        "IBMPlexMono/IBMPlexMono-Regular.ttf",
        "IBMPlexMono/IBMPlexMono-Medium.ttf",
        "IBMPlexMono/IBMPlexMono-SemiBold.ttf",
        "SpaceGrotesk/SpaceGrotesk-Regular.ttf",
        "SpaceGrotesk/SpaceGrotesk-Medium.ttf",
        "SpaceGrotesk/SpaceGrotesk-SemiBold.ttf",
        "SpaceGrotesk/SpaceGrotesk-Bold.ttf",
    };
    for (const auto& f : files) QFontDatabase::addApplicationFont(dir + "/" + f);
}

enum HistCol { ColExprIdx = 0, ColResultIdx, ColWifIdx, ColPubIdx, ColAddressIdx, ColCount };

} // namespace

class BigCalcWindow : public QMainWindow {
public:
    BigCalcWindow() {
        buildUi();
        applyTheme(g_theme);
        applyTranslations();
        hPrec->setText("3");
        hLeft->setText("0xFF");
        hRight->setText("1.5");
        syncExprFromParts();
    }

private:
    QLineEdit* hLeft = nullptr;
    QComboBox* hOp = nullptr;
    QLineEdit* hRight = nullptr;
    QLineEdit* hPrec = nullptr;
    QLineEdit* hExpr = nullptr;
    QPushButton* hBtnCalc = nullptr;
    QPushButton* hBtnClear = nullptr;
    QPlainTextEdit* hDec = nullptr;
    QPlainTextEdit* hHex = nullptr;
    QTableWidget* hHist = nullptr;
    QLabel* hLblLang = nullptr;
    QComboBox* hComboLang = nullptr;
    QLabel* hLblTheme = nullptr;
    QComboBox* hComboTheme = nullptr;

    QLabel* hLblAppTitle = nullptr;
    QLabel* hLblAppSubtitle = nullptr;
    QLabel* hLblLeft = nullptr;
    QLabel* hLblOp = nullptr;
    QLabel* hLblRight = nullptr;
    QLabel* hLblPrec = nullptr;
    QLabel* hLblExpr = nullptr;
    QLabel* hLblDec = nullptr;
    QLabel* hLblHex = nullptr;
    QLabel* hLblHist = nullptr;

    QWidget* hDecPanel = nullptr;
    QWidget* hHexPanel = nullptr;
    QWidget* hHistPanel = nullptr;

    void buildUi() {
        auto* central = new QWidget(this);
        central->setObjectName("central");
        auto* rootLayout = new QVBoxLayout(central);

        // --- header: app title/subtitle + Language / Appearance switchers ---
        auto* headerRow = new QHBoxLayout();
        auto* titleCol = new QVBoxLayout();
        hLblAppTitle = new QLabel("BigCalc");
        hLblAppTitle->setObjectName("appTitle");
        hLblAppSubtitle = new QLabel();
        hLblAppSubtitle->setObjectName("appSubtitle");
        titleCol->addWidget(hLblAppTitle);
        titleCol->addWidget(hLblAppSubtitle);
        headerRow->addLayout(titleCol);
        headerRow->addStretch(1);

        auto* langCol = new QVBoxLayout();
        hLblLang = new QLabel();
        hLblLang->setObjectName("smallLabel");
        hComboLang = new QComboBox();
        hComboLang->addItems({"Magyar", "English", "Deutsch"});
        hComboLang->setCurrentIndex(static_cast<int>(g_lang));
        langCol->addWidget(hLblLang);
        langCol->addWidget(hComboLang);
        headerRow->addLayout(langCol);
        headerRow->addSpacing(12);

        auto* themeCol = new QVBoxLayout();
        hLblTheme = new QLabel();
        hLblTheme->setObjectName("smallLabel");
        hComboTheme = new QComboBox();
        hComboTheme->addItem("", static_cast<int>(Theme::TerminalDark));
        hComboTheme->addItem("", static_cast<int>(Theme::LightDashboard));
        hComboTheme->addItem("", static_cast<int>(Theme::DarkFintech));
        hComboTheme->setCurrentIndex(static_cast<int>(g_theme));
        themeCol->addWidget(hLblTheme);
        themeCol->addWidget(hComboTheme);
        headerRow->addLayout(themeCol);

        rootLayout->addLayout(headerRow);

        // --- controls row: Left / Op / Right / Prec ---
        auto* topRow = new QHBoxLayout();
        auto* leftCol = new QVBoxLayout();
        hLblLeft = new QLabel();
        hLblLeft->setObjectName("fieldLabel");
        hLeft = new QLineEdit();
        leftCol->addWidget(hLblLeft);
        leftCol->addWidget(hLeft);

        auto* opCol = new QVBoxLayout();
        hLblOp = new QLabel();
        hLblOp->setObjectName("fieldLabel");
        hOp = new QComboBox();
        hOp->addItems({"+", "-", "*", "/"});
        hOp->setCurrentIndex(2);
        opCol->addWidget(hLblOp);
        opCol->addWidget(hOp);

        auto* rightCol = new QVBoxLayout();
        hLblRight = new QLabel();
        hLblRight->setObjectName("fieldLabel");
        hRight = new QLineEdit();
        rightCol->addWidget(hLblRight);
        rightCol->addWidget(hRight);

        auto* precCol = new QVBoxLayout();
        hLblPrec = new QLabel();
        hLblPrec->setObjectName("fieldLabel");
        hPrec = new QLineEdit();
        precCol->addWidget(hLblPrec);
        precCol->addWidget(hPrec);

        topRow->addLayout(leftCol, 45);
        topRow->addLayout(opCol, 12);
        topRow->addLayout(rightCol, 30);
        topRow->addLayout(precCol, 15);
        rootLayout->addLayout(topRow);

        // --- expression row ---
        hLblExpr = new QLabel();
        hLblExpr->setObjectName("fieldLabel");
        rootLayout->addWidget(hLblExpr);
        auto* exprRow = new QHBoxLayout();
        hExpr = new QLineEdit();
        hBtnCalc = new QPushButton();
        hBtnCalc->setObjectName("btnPrimary");
        hBtnClear = new QPushButton();
        hBtnClear->setObjectName("btnSecondary");
        exprRow->addWidget(hExpr, 1);
        exprRow->addWidget(hBtnCalc);
        exprRow->addWidget(hBtnClear);
        rootLayout->addLayout(exprRow);

        // --- three resizable panels: Decimal / Hex / History ---
        auto* splitter = new QSplitter(Qt::Vertical);
        splitter->setObjectName("panelSplitter");

        hDecPanel = new QWidget();
        hDecPanel->setObjectName("panelCard");
        auto* decLayout = new QVBoxLayout(hDecPanel);
        hLblDec = new QLabel();
        hLblDec->setObjectName("panelTitle");
        hDec = new QPlainTextEdit();
        hDec->setObjectName("resultBox");
        hDec->setReadOnly(true);
        decLayout->addWidget(hLblDec);
        decLayout->addWidget(hDec);
        splitter->addWidget(hDecPanel);

        hHexPanel = new QWidget();
        hHexPanel->setObjectName("panelCard");
        auto* hexLayout = new QVBoxLayout(hHexPanel);
        hLblHex = new QLabel();
        hLblHex->setObjectName("panelTitle");
        hHex = new QPlainTextEdit();
        hHex->setObjectName("resultBox");
        hHex->setReadOnly(true);
        hexLayout->addWidget(hLblHex);
        hexLayout->addWidget(hHex);
        splitter->addWidget(hHexPanel);

        hHistPanel = new QWidget();
        hHistPanel->setObjectName("panelCard");
        auto* histLayout = new QVBoxLayout(hHistPanel);
        hLblHist = new QLabel();
        hLblHist->setObjectName("panelTitle");
        hHist = new QTableWidget(0, ColCount);
        hHist->setEditTriggers(QAbstractItemView::NoEditTriggers);
        hHist->setSelectionBehavior(QAbstractItemView::SelectItems);
        hHist->setSelectionMode(QAbstractItemView::SingleSelection);
        hHist->horizontalHeader()->setStretchLastSection(true);
        hHist->setColumnWidth(ColExprIdx, 170);
        hHist->setColumnWidth(ColResultIdx, 150);
        hHist->setColumnWidth(ColWifIdx, 260);
        hHist->setColumnWidth(ColPubIdx, 260);
        hHist->verticalHeader()->setVisible(false);
        hHist->setContextMenuPolicy(Qt::CustomContextMenu);
        histLayout->addWidget(hLblHist);
        histLayout->addWidget(hHist);
        splitter->addWidget(hHistPanel);

        splitter->setSizes({360, 360, 280});
        rootLayout->addWidget(splitter, 1);

        setCentralWidget(central);
        statusBar();

        setMinimumSize(820, 600);
        resize(1080, 780);

        // --- signals ---
        connect(hBtnCalc, &QPushButton::clicked, this, [this]() {
            bool fromParts = !hExpr->hasFocus();
            runCalculate(fromParts, true);
        });
        connect(hBtnClear, &QPushButton::clicked, this, [this]() { onClear(); });
        connect(hComboLang, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this](int idx) {
                    if (idx < 0 || idx > 2) return;
                    g_lang = static_cast<Lang>(idx);
                    saveLangToIni();
                    applyTranslations();
                });
        connect(hComboTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this](int idx) {
                    if (idx < 0 || idx > 2) return;
                    Theme t = static_cast<Theme>(idx);
                    saveThemeToIni();
                    applyTheme(t);
                });
        connect(hHist, &QTableWidget::customContextMenuRequested, this,
                [this](const QPoint& pos) { showHistoryContextMenu(pos); });

        auto* copyShortcut = new QShortcut(QKeySequence::Copy, hHist);
        connect(copyShortcut, &QShortcut::activated, this, [this]() {
            int row = hHist->currentRow();
            int col = hHist->currentColumn();
            if (col < 0) col = ColResultIdx;
            if (row >= 0) copyHistoryCell(row, col);
        });
    }

    // ---------------------------------------------------------------

    static QString widen(const std::string& s) { return QString::fromStdString(s); }
    static std::string narrow(const QString& s) { return s.toStdString(); }

    void setStatus(const QString& s) { statusBar()->showMessage(s); }

    void syncExprFromParts() {
        QString left = hLeft->text();
        QString right = hRight->text();
        QString op = hOp->currentText();
        hExpr->setText(left + " " + op + " " + right);
    }

    int selectedPrecision() const {
        bool ok = false;
        int v = hPrec->text().toInt(&ok);
        return ok ? v : 3;
    }

    void runCalculate(bool fromParts, bool showPopup) {
        std::string expr;
        if (fromParts) {
            syncExprFromParts();
            expr = narrow(hExpr->text());
        } else {
            expr = narrow(hExpr->text());
            std::string left, right;
            char op = 0;
            if (splitExpression(expr, left, op, right)) {
                hLeft->setText(widen(left));
                hRight->setText(widen(right));
                const char ops[] = {'+', '-', '*', '/'};
                for (int i = 0; i < 4; ++i) {
                    if (ops[i] == op) {
                        hOp->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }

        int prec = selectedPrecision();
        UnifiedResult r = evaluateSmartExpression(expr, prec);

        if (!r.ok) {
            hDec->setPlainText("");
            hHex->setPlainText("");
            setStatus(widen(T(Msg::StatusErrorPrefix)) + widen(r.error));
            if (showPopup)
                QMessageBox::warning(this, widen(T(Msg::MsgBoxTitle)), widen(r.error));
            return;
        }

        if (r.isPointResult) {
            hDec->setPlainText(widen("[" + r.mode + "] " + r.decimal + "\nX=" + r.pointX + "\nY=" + r.pointY));
            hHex->setPlainText(widen(r.hex));
        } else {
            hDec->setPlainText(widen("[" + r.mode + "] " + r.decimal));
            hHex->setPlainText(widen(r.hex));
        }

        BitcoinKeyInfo btc;
        if (r.isPointResult && !r.pointCompressed.empty()) {
            btc = deriveFromCompressedPub(r.pointCompressed);
        } else if (!r.intDecimal.empty()) {
            btc = deriveBitcoinKeys(r.intDecimal);
        }

        QString wif = btc.ok ? widen(btc.wif) : "-";
        QString pub = btc.ok ? widen(btc.pubCompressed) : (r.isPointResult ? widen(r.pointCompressed) : "-");
        QString addr = btc.ok ? widen(btc.address) : "-";
        QString resultCol = r.isPointResult ? widen(r.pointCompressed) : widen(r.decimal);
        if (!btc.ok && !btc.error.empty())
            resultCol += "  [" + widen(btc.error) + "]";

        int row = 0;
        hHist->insertRow(row);
        hHist->setItem(row, ColExprIdx, new QTableWidgetItem(widen(r.expression)));
        hHist->setItem(row, ColResultIdx, new QTableWidgetItem(resultCol));
        hHist->setItem(row, ColWifIdx, new QTableWidgetItem(wif));
        hHist->setItem(row, ColPubIdx, new QTableWidgetItem(pub));
        hHist->setItem(row, ColAddressIdx, new QTableWidgetItem(addr));

        if (!r.syncMessage.empty())
            setStatus(widen(r.syncMessage));
        else
            setStatus(widen(T(Msg::StatusModePrefix)) + widen(r.mode) +
                      (btc.ok ? (widen(T(Msg::StatusAddressSuffix)) + addr) : QString()));
    }

    void onClear() {
        hLeft->setText("");
        hRight->setText("");
        hExpr->setText("");
        hDec->setPlainText("");
        hHex->setPlainText("");
        hHist->setRowCount(0);
        setStatus(widen(T(Msg::StatusFieldsCleared)));
    }

    QString historyColName(int col) const {
        switch (col) {
        case ColExprIdx: return widen(T(Msg::ColExpr));
        case ColResultIdx: return widen(T(Msg::ColResult));
        case ColWifIdx: return widen(T(Msg::ColWif));
        case ColPubIdx: return widen(T(Msg::ColPub));
        case ColAddressIdx: return widen(T(Msg::ColAddress));
        default: return widen(T(Msg::ColCell));
        }
    }

    QString historyCellText(int row, int col) const {
        auto* item = hHist->item(row, col);
        return item ? item->text() : QString();
    }

    void copyHistoryCell(int row, int col) {
        QString text = historyCellText(row, col);
        if (text.isEmpty() || text == "-") {
            setStatus(widen(T(Msg::StatusNoCopyableData)));
            return;
        }
        QGuiApplication::clipboard()->setText(text);
        setStatus(widen(T(Msg::StatusCopiedPrefix)) + historyColName(col) +
                  widen(T(Msg::StatusCopiedSuffix)) + text);
    }

    void copyHistoryRow(int row) {
        if (row < 0) return;
        QStringList parts;
        for (int c = 0; c < ColCount; ++c) parts << historyCellText(row, c);
        QGuiApplication::clipboard()->setText(parts.join('\t'));
        setStatus(widen(T(Msg::StatusFullRowCopied)));
    }

    void showHistoryContextMenu(const QPoint& pos) {
        auto* item = hHist->itemAt(pos);
        int row = item ? item->row() : hHist->currentRow();
        int col = item ? item->column() : hHist->currentColumn();
        if (row < 0) return;
        if (item) hHist->setCurrentCell(row, col);

        QMenu menu(this);
        QAction* copyCell = menu.addAction(widen(T(Msg::CtxCopyCellPrefix)) + historyColName(col));
        QAction* copyRow = menu.addAction(widen(T(Msg::CtxCopyRow)));
        QAction* chosen = menu.exec(hHist->viewport()->mapToGlobal(pos));
        if (chosen == copyCell) copyHistoryCell(row, col);
        else if (chosen == copyRow) copyHistoryRow(row);
    }

    void applyTranslations() {
        setWindowTitle(widen(T(Msg::WinTitle)));
        hLblAppSubtitle->setText(widen(T(Msg::AppSubtitle)));
        hLblLeft->setText(widen(T(Msg::LblLeft)));
        hLblOp->setText(widen(T(Msg::LblOp)));
        hLblRight->setText(widen(T(Msg::LblRight)));
        hLblPrec->setText(widen(T(Msg::LblPrec)));
        hLblExpr->setText(widen(T(Msg::LblExpr)));
        hLblDec->setText(widen(T(Msg::LblDec)));
        hLblHex->setText(widen(T(Msg::LblHex)));
        hLblHist->setText(widen(T(Msg::LblHist)));
        hLblLang->setText(widen(T(Msg::LangLabel)));
        hLblTheme->setText(widen(T(Msg::ThemeLabel)));
        hBtnCalc->setText(widen(T(Msg::BtnCalc)));
        hBtnClear->setText(widen(T(Msg::BtnClear)));

        {
            QSignalBlocker block(hComboTheme);
            hComboTheme->setItemText(static_cast<int>(Theme::TerminalDark), widen(T(Msg::ThemeTerminal)));
            hComboTheme->setItemText(static_cast<int>(Theme::LightDashboard), widen(T(Msg::ThemeLight)));
            hComboTheme->setItemText(static_cast<int>(Theme::DarkFintech), widen(T(Msg::ThemeFintech)));
        }

        hHist->setHorizontalHeaderLabels({
            historyColName(ColExprIdx), historyColName(ColResultIdx),
            historyColName(ColWifIdx), historyColName(ColPubIdx),
            historyColName(ColAddressIdx)
        });
        setStatus(widen(T(Msg::StatusReady)));
    }

    // ---------------------------------------------------------------
    // Megjelenes (skin) valto: 3 QSS tema + betutipus-parositas.
    // ---------------------------------------------------------------

    void applyTheme(Theme t) {
        g_theme = t;
        {
            QSignalBlocker block(hComboTheme);
            hComboTheme->setCurrentIndex(static_cast<int>(t));
        }
        setStyleSheet(styleSheetFor(t));
        applyPanelShadows(t);
    }

    void applyPanelShadows(Theme t) {
        // Csak a vilagos temaban van lagy kartya-arnyek; a sotet temakban
        // a border viszi a kartya-erzetet (arnyek ott alig latszik).
        const bool wantShadow = (t == Theme::LightDashboard);
        for (QWidget* panel : {hDecPanel, hHexPanel, hHistPanel}) {
            if (!wantShadow) {
                panel->setGraphicsEffect(nullptr);
                continue;
            }
            auto* shadow = new QGraphicsDropShadowEffect(panel);
            shadow->setBlurRadius(24);
            shadow->setOffset(0, 4);
            shadow->setColor(QColor(40, 44, 68, 45));
            panel->setGraphicsEffect(shadow);
        }
    }

    static QString styleSheetFor(Theme t) {
        switch (t) {
        case Theme::LightDashboard: return lightDashboardQss();
        case Theme::DarkFintech: return darkFintechQss();
        default: return terminalDarkQss();
        }
    }

    static QString terminalDarkQss() {
        return R"CSS(
            QMainWindow, QWidget#central { background: #171d1a; }
            QWidget { color: #cfead9; font-family: 'JetBrains Mono'; font-size: 13px; }
            QLabel#appTitle { font-size: 20px; font-weight: 800; letter-spacing: 2px; color: #39ff88; }
            QLabel#appSubtitle { font-size: 11px; color: #6f8f7c; }
            QLabel#smallLabel { font-size: 10px; color: #6f8f7c; letter-spacing: 1px; }
            QLabel#fieldLabel { font-size: 10.5px; color: #6f8f7c; }
            QLabel#panelTitle { font-size: 11.5px; font-weight: 700; letter-spacing: 1px; color: #39ff88; }
            QLineEdit, QPlainTextEdit, QTableWidget {
                background: #101513; border: 1px solid rgba(120,190,150,0.45);
                border-radius: 3px; color: #cfead9; padding: 6px 8px; selection-background-color: #2a5b3f;
            }
            QLineEdit:focus { border: 1px solid #39ff88; }
            QComboBox {
                background: #1c2622; border: 1px solid rgba(120,190,150,0.45);
                border-radius: 3px; color: #cfead9; padding: 5px 8px;
            }
            QComboBox QAbstractItemView {
                background: #1c2622; color: #cfead9; selection-background-color: #2a5b3f; outline: none;
            }
            QPushButton#btnPrimary {
                background: #39ff88; color: #0c1310; font-weight: 800; border: none;
                border-radius: 3px; padding: 8px 20px;
            }
            QPushButton#btnPrimary:hover { background: #6dffa9; }
            QPushButton#btnSecondary {
                background: transparent; color: #9db8a9; border: 1px solid rgba(120,190,150,0.45);
                border-radius: 3px; padding: 8px 18px; font-weight: 700;
            }
            QPushButton#btnSecondary:hover { border-color: #39ff88; color: #cfead9; }
            QWidget#panelCard { background: #101513; border: 1px solid rgba(120,190,150,0.45); border-radius: 3px; }
            QHeaderView::section {
                background: #1c2622; color: #9db8a9; border: none; border-bottom: 1px solid rgba(120,190,150,0.35);
                padding: 6px; font-weight: 700; font-size: 10.5px;
            }
            QTableWidget { gridline-color: rgba(120,190,150,0.15); border: 1px solid rgba(120,190,150,0.45); }
            QTableWidget::item { padding: 3px 4px; }
            QTableWidget::item:selected { background: rgba(57,255,136,0.28); color: #eafff2; }
            QSplitter::handle { background: rgba(120,190,150,0.25); }
            QStatusBar { background: transparent; color: #9db8a9; border-top: 1px solid rgba(120,190,150,0.35); }
        )CSS";
    }

    static QString lightDashboardQss() {
        return R"CSS(
            QMainWindow, QWidget#central { background: #f6f7fb; }
            QWidget { color: #33384a; font-family: 'Manrope'; font-size: 13px; }
            QLabel#appTitle { font-size: 20px; font-weight: 800; color: #23273a; font-family: 'Manrope'; }
            QLabel#appSubtitle { font-size: 12px; color: #6a6f82; }
            QLabel#smallLabel { font-size: 10.5px; font-weight: 700; color: #6a6f82; }
            QLabel#fieldLabel { font-size: 11px; font-weight: 600; color: #666c80; }
            QLabel#panelTitle { font-size: 11.5px; font-weight: 700; letter-spacing: 0.5px; color: #6a6f82; }
            QLineEdit, QPlainTextEdit, QTableWidget {
                background: #ffffff; border: 1px solid #dde0ea; border-radius: 8px;
                color: #2c3042; padding: 7px 10px; font-family: 'IBM Plex Mono'; selection-background-color: #dfe3ff;
            }
            QLineEdit:focus { border: 1.5px solid #4f5fd1; }
            QComboBox {
                background: #ffffff; border: 1px solid #dde0ea; border-radius: 8px;
                color: #2c3042; padding: 6px 10px; font-family: 'Manrope';
            }
            QComboBox QAbstractItemView {
                background: #ffffff; color: #2c3042; selection-background-color: #eef0ff; outline: none;
            }
            QPushButton#btnPrimary {
                background: #4f5fd1; color: #ffffff; font-weight: 700; border: none;
                border-radius: 8px; padding: 8px 22px; font-family: 'Manrope';
            }
            QPushButton#btnPrimary:hover { background: #5f6ee0; }
            QPushButton#btnSecondary {
                background: #ffffff; color: #4b5164; border: 1px solid #d8dce6;
                border-radius: 8px; padding: 8px 20px; font-weight: 700; font-family: 'Manrope';
            }
            QPushButton#btnSecondary:hover { border-color: #b7bcd6; }
            QWidget#panelCard { background: #ffffff; border: 1px solid #e6e8f0; border-radius: 12px; }
            QHeaderView::section {
                background: #f6f7fb; color: #6a6f82; border: none; border-bottom: 1px solid #e6e8f0;
                padding: 6px; font-weight: 700; font-size: 10.5px; font-family: 'Manrope';
            }
            QTableWidget { gridline-color: #eef0f5; border: 1px solid #e6e8f0; }
            QTableWidget::item { padding: 3px 4px; }
            QTableWidget::item:selected { background: #e8eaff; color: #2c3042; }
            QSplitter::handle { background: #e6e8f0; }
            QStatusBar { background: transparent; color: #5b6175; border-top: 1px solid #e6e8f0; }
        )CSS";
    }

    static QString darkFintechQss() {
        return R"CSS(
            QMainWindow, QWidget#central { background: #1b2130; }
            QWidget { color: #dde1ea; font-family: 'Space Grotesk'; font-size: 13px; }
            QLabel#appTitle { font-size: 20px; font-weight: 700; color: #f0f1f5; }
            QLabel#appSubtitle { font-size: 12px; color: #929aad; }
            QLabel#smallLabel { font-size: 10.5px; font-weight: 600; color: #929aad; }
            QLabel#fieldLabel { font-size: 11px; font-weight: 600; color: #8b93a6; }
            QLabel#panelTitle { font-size: 11.5px; font-weight: 700; letter-spacing: 0.5px; color: #e0b25a; }
            QLineEdit, QPlainTextEdit, QTableWidget {
                background: #222939; border: 1px solid #39415a; border-radius: 7px;
                color: #e7ebf1; padding: 7px 10px; font-family: 'IBM Plex Mono'; selection-background-color: #4a3c22;
            }
            QLineEdit:focus { border: 1.5px solid #e0b25a; }
            QComboBox {
                background: #262e40; border: 1px solid #39415a; border-radius: 7px;
                color: #dde1ea; padding: 6px 10px; font-family: 'Space Grotesk';
            }
            QComboBox QAbstractItemView {
                background: #262e40; color: #dde1ea; selection-background-color: #3a3220; outline: none;
            }
            QPushButton#btnPrimary {
                background: #e0b25a; color: #1b2130; font-weight: 700; border: none;
                border-radius: 7px; padding: 8px 22px; font-family: 'Space Grotesk';
            }
            QPushButton#btnPrimary:hover { background: #eec27a; }
            QPushButton#btnSecondary {
                background: transparent; color: #a7adba; border: 1px solid #3f485d;
                border-radius: 7px; padding: 8px 20px; font-weight: 700; font-family: 'Space Grotesk';
            }
            QPushButton#btnSecondary:hover { border-color: #e0b25a; color: #dde1ea; }
            QWidget#panelCard { background: #222939; border: 1px solid #333c50; border-radius: 10px; }
            QHeaderView::section {
                background: #262e40; color: #929aad; border: none; border-bottom: 1px solid #333c50;
                padding: 6px; font-weight: 700; font-size: 10.5px; font-family: 'Space Grotesk';
            }
            QTableWidget { gridline-color: #333c50; border: 1px solid #333c50; }
            QTableWidget::item { padding: 3px 4px; }
            QTableWidget::item:selected { background: rgba(224,178,90,0.22); color: #f3e6c8; }
            QSplitter::handle { background: #333c50; }
            QStatusBar { background: transparent; color: #929aad; border-top: 1px solid #333c50; }
        )CSS";
    }
};

int main(int argc, char** argv) {
    QApplication::setStyle("Fusion");
    QApplication app(argc, argv);
    loadBundledFonts();
    loadLangFromIni();
    loadThemeFromIni();
    BigCalcWindow window;
    window.show();
    return app.exec();
}
