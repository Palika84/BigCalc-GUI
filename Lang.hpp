#pragma once
// Egyszeru futasidejü UI-nyelv valto (HU/EN/DE). Minden felhasznalonak lathato
// szoveg (GUI chrome + motor hiba-/allapotuzenetek) ezen a tablan keresztul megy.
//
// Hasznalat:  T(Msg::XyzKey)  ->  std::string az aktualis g_lang nyelven.
// main.cpp-ben a widen(T(...)) mintaval kerul at wchar_t-re a Win32 hivasokhoz.

#include <array>
#include <cstddef>
#include <string>

enum class Lang { HU = 0, EN = 1, DE = 2 };

// Alapertelmezett: magyar. A GUI nyelvvalasztoja es a bigcalc_lang.ini allitja at.
inline Lang g_lang = Lang::HU;

enum class Msg {
    // --- Calculator.hpp ---
    CalcEmptyNumber,
    CalcInvalidHexFormat,
    CalcInvalidHexDigit,
    CalcInvalidDecimalFormat,
    CalcPrecisionNegative,
    CalcPrecisionMax,
    CalcParseExpressionFailed,
    CalcDivisionByZero,
    CalcUnknownOperator,

    // --- BigInt.hpp ---
    BigIntDivisionByZero,
    BigIntNegativeExponent,
    BigIntEmptyNumber,
    BigIntInvalidDecimal,
    BigIntInvalidDecimalDigit,
    BigIntInvalidHexDigit,

    // --- ModeEngine.hpp ---
    ModeDec,
    ModeHexScalar,
    ModeGPoint,
    ModeGCurve,
    ModeGRangeCount,
    ErrGModScalarPositive,
    ErrGModInfinity,
    ErrUnknownOperandG,
    NoteInfinityPoint,
    NoteGPointXPrefix,
    ErrLeftOperandInvalid,
    ErrRightOperandInvalid,
    NoteKTimesP,
    NotePTimesK,
    NoteGModePrefix,
    ErrGModMulNeedsScalarPoint,
    ErrRangeCountTargetPrefix,
    ErrRangeCountStartPrefix,
    ErrGModDivScalarPointUndefined,
    ErrGModDivisorPositive,
    ErrGModSlashGenericPrefix,
    NotePDivKFormula,
    NoteABDivGFormula,
    ErrGModDivNeedsRangeCountOrScalar,
    ErrGModSupportedOps,
    ErrLeftGPrefix,
    ErrRightGPrefix,
    NoteP1PlusNegP2Prefix,
    NoteGPointResultPrefix,
    SuffixScalarToG,
    ErrParseExpressionGeneric,
    TagScalarPoint,
    TagPointScalar,
    TagKMTimesG,
    TagRangeCountTargetStart,
    TagPointDivScalar,
    TagABDivG,
    TagP1PlusNegP2,
    TagP1PlusP2,

    // --- crypto/BitcoinCrypto.cpp ---
    BtcEmptyPrivKey,
    BtcNegativePrivKey,
    BtcEmptyHexKey,
    BtcInvalidHexDigitInKey,
    BtcKeyLargerThan256,
    BtcInvalidDecimalKey,
    BtcPrivKeyZero,
    BtcPrivKeyGERange,
    BtcPubkeyInfinityInvalidScalar,
    BtcCompressedPubNeeds66Hex,

    // --- crypto/SecpAffine.cpp ---
    SecpHexCoordOver256,
    SecpParityInfinity,
    SecpParityOdd,
    SecpParityEven,
    SecpEmptyHexCoord,
    SecpInvalidHexDigit,
    SecpCoordOver256,
    SecpInvNoInverseP,
    SecpInvModPFailed,
    SecpInvNoInverseN,
    SecpInvModNFailed,
    RangeCountStartInfinity,
    RangeCountTargetInfinity,
    RangeCountFoundZeroSteps,
    RangeCountFoundStepsPrefix,
    RangeCountFoundStepsSuffix,
    RangeCountNotFoundPrefix,
    RangeCountNotFoundSuffix,
    CompressedPubLength66,
    CompressedPrefix0203,
    XCoordNotInField,
    NoSquareRootNotOnCurve,
    UncompressedNotOnCurve,
    UnknownPubkeyForm,

    // --- main.cpp (GUI chrome) ---
    WinTitle,
    LblLeft,
    LblOp,
    LblRight,
    LblPrec,
    LblExpr,
    LblDec,
    LblHex,
    LblHist,
    BtnCalc,
    BtnClear,
    StatusReady,
    ColExpr,
    ColResult,
    ColWif,
    ColPub,
    ColAddress,
    ColCell,
    CtxCopyCellPrefix,
    CtxCopyRow,
    StatusErrorPrefix,
    MsgBoxTitle,
    StatusModePrefix,
    StatusAddressSuffix,
    StatusNoCopyableData,
    StatusCopiedPrefix,
    StatusCopiedSuffix,
    StatusCopyFailed,
    StatusFullRowCopied,
    StatusFieldsCleared,
    StatusSelectedPrefix,
    StatusSelectedSuffix,
    LangLabel,

    _Count
};

struct Tri { const char* hu; const char* en; const char* de; };

inline const std::array<Tri, static_cast<size_t>(Msg::_Count)>& msgTable() {
    static const std::array<Tri, static_cast<size_t>(Msg::_Count)> t = {{
        /* CalcEmptyNumber */                  {"Üres szám.", "Empty number.", "Leere Zahl."},
        /* CalcInvalidHexFormat */              {"Érvénytelen hex formátum.", "Invalid hex format.", "Ungültiges Hex-Format."},
        /* CalcInvalidHexDigit */               {"Érvénytelen hex jegy.", "Invalid hex digit.", "Ungültige Hex-Ziffer."},
        /* CalcInvalidDecimalFormat */          {"Érvénytelen decimális formátum.", "Invalid decimal format.", "Ungültiges Dezimalformat."},
        /* CalcPrecisionNegative */             {"A pontosság nem lehet negatív.", "Precision cannot be negative.", "Die Genauigkeit darf nicht negativ sein."},
        /* CalcPrecisionMax */                  {"A pontosság maximum 64 lehet.", "Precision can be at most 64.", "Die Genauigkeit darf höchstens 64 sein."},
        /* CalcParseExpressionFailed */         {"Nem sikerült értelmezni a kifejezést. Példa: 0xFF * 1.5", "Could not parse the expression. Example: 0xFF * 1.5", "Der Ausdruck konnte nicht verarbeitet werden. Beispiel: 0xFF * 1.5"},
        /* CalcDivisionByZero */                {"Nullával való osztás.", "Division by zero.", "Division durch null."},
        /* CalcUnknownOperator */               {"Ismeretlen operátor.", "Unknown operator.", "Unbekannter Operator."},

        /* BigIntDivisionByZero */              {"Nullával való osztás.", "Division by zero.", "Division durch null."},
        /* BigIntNegativeExponent */            {"Negatív kitevő.", "Negative exponent.", "Negativer Exponent."},
        /* BigIntEmptyNumber */                 {"Üres szám.", "Empty number.", "Leere Zahl."},
        /* BigIntInvalidDecimal */              {"Érvénytelen decimális szám.", "Invalid decimal.", "Ungültige Dezimalzahl."},
        /* BigIntInvalidDecimalDigit */         {"Érvénytelen decimális jegy.", "Invalid decimal digit.", "Ungültige Dezimalziffer."},
        /* BigIntInvalidHexDigit */             {"Érvénytelen hex jegy.", "Invalid hex digit.", "Ungültige Hex-Ziffer."},

        /* ModeDec */                           {"DEC", "DEC", "DEZ"},
        /* ModeHexScalar */                     {"HEX/SKALÁR", "HEX/SCALAR", "HEX/SKALAR"},
        /* ModeGPoint */                        {"G/PONT", "G/POINT", "G/PUNKT"},
        /* ModeGCurve */                        {"G/GÖRBE", "G/CURVE", "G/KURVE"},
        /* ModeGRangeCount */                   {"G/RANGE-COUNT", "G/RANGE-COUNT", "G/RANGE-COUNT"},
        /* ErrGModScalarPositive */             {"G-mód: a skalár legyen pozitív egész (k*G).", "G-mode: the scalar must be a positive integer (k*G).", "G-Modus: der Skalar muss eine positive Ganzzahl sein (k*G)."},
        /* ErrGModInfinity */                   {"G-mód: k*G infinity.", "G-mode: k*G is infinity.", "G-Modus: k*G ist infinity."},
        /* ErrUnknownOperandG */                {"Ismeretlen operandus G-módhoz.", "Unknown operand for G-mode.", "Unbekannter Operand für den G-Modus."},
        /* NoteInfinityPoint */                 {"G-mód: VÉGTELEN PONT (infinity)", "G-mode: POINT AT INFINITY", "G-Modus: PUNKT IM UNENDLICHEN"},
        /* NoteGPointXPrefix */                 {"G-mód pont  X=", "G-mode point  X=", "G-Modus-Punkt  X="},
        /* ErrLeftOperandInvalid */             {"Bal operandus érvénytelen: ", "Left operand invalid: ", "Linker Operand ungültig: "},
        /* ErrRightOperandInvalid */            {"Jobb operandus érvénytelen: ", "Right operand invalid: ", "Rechter Operand ungültig: "},
        /* NoteKTimesP */                       {"G-mód: k*P", "G-mode: k*P", "G-Modus: k*P"},
        /* NotePTimesK */                       {"G-mód: P*k", "G-mode: P*k", "G-Modus: P*k"},
        /* NoteGModePrefix */                   {"G-mód: ", "G-mode: ", "G-Modus: "},
        /* ErrGModMulNeedsScalarPoint */        {"G-mód *: skalár*pont vagy pont*skalár kell.", "G-mode *: needs scalar*point or point*scalar.", "G-Modus *: erfordert Skalar*Punkt oder Punkt*Skalar."},
        /* ErrRangeCountTargetPrefix */         {"Range-Count Cél: ", "Range-Count Target: ", "Range-Count Ziel: "},
        /* ErrRangeCountStartPrefix */          {"Range-Count Start: ", "Range-Count Start: ", "Range-Count Start: "},
        /* ErrGModDivScalarPointUndefined */    {"G-mód /: skalár/pont nem értelmezett. Használj: CélPont / StartPont (Range-Count) vagy pont / skalár.", "G-mode /: scalar/point is not defined. Use: TargetPoint / StartPoint (Range-Count) or point / scalar.", "G-Modus /: Skalar/Punkt ist nicht definiert. Verwende: ZielPunkt / StartPunkt (Range-Count) oder Punkt / Skalar."},
        /* ErrGModDivisorPositive */            {"G-mód /: az osztó legyen pozitív egész skalár.", "G-mode /: the divisor must be a positive integer scalar.", "G-Modus /: der Divisor muss ein positiver ganzzahliger Skalar sein."},
        /* ErrGModSlashGenericPrefix */         {"G-mód /: ", "G-mode /: ", "G-Modus /: "},
        /* NotePDivKFormula */                  {"G-mód: P/k = (k^{-1} mod n)*P", "G-mode: P/k = (k^{-1} mod n)*P", "G-Modus: P/k = (k^{-1} mod n)*P"},
        /* NoteABDivGFormula */                 {"G-mód: (a*b^{-1} mod n)*G", "G-mode: (a*b^{-1} mod n)*G", "G-Modus: (a*b^{-1} mod n)*G"},
        /* ErrGModDivNeedsRangeCountOrScalar */ {"G-mód /: CélPont/StartPont (Range-Count) vagy pont/skalár kell.", "G-mode /: needs TargetPoint/StartPoint (Range-Count) or point/scalar.", "G-Modus /: erfordert ZielPunkt/StartPunkt (Range-Count) oder Punkt/Skalar."},
        /* ErrGModSupportedOps */               {"G-módban +, -, * (skalár*pont) és / (Range-Count vagy pont/skalár) támogatott.", "In G-mode, +, -, * (scalar*point) and / (Range-Count or point/scalar) are supported.", "Im G-Modus werden +, -, * (Skalar*Punkt) und / (Range-Count oder Punkt/Skalar) unterstützt."},
        /* ErrLeftGPrefix */                    {"Bal (G): ", "Left (G): ", "Links (G): "},
        /* ErrRightGPrefix */                   {"Jobb (G): ", "Right (G): ", "Rechts (G): "},
        /* NoteP1PlusNegP2Prefix */             {"G-mód: P1 + (-P2)  X=", "G-mode: P1 + (-P2)  X=", "G-Modus: P1 + (-P2)  X="},
        /* NoteGPointResultPrefix */            {"G-mód pont  X=", "G-mode point  X=", "G-Modus-Punkt  X="},
        /* SuffixScalarToG */                   {"  (skalár→k*G)", "  (scalar→k*G)", "  (Skalar→k*G)"},
        /* ErrParseExpressionGeneric */         {"Nem sikerült értelmezni a kifejezést.", "Could not parse the expression.", "Der Ausdruck konnte nicht verarbeitet werden."},
        /* TagScalarPoint */                    {"skalár*pont", "scalar*point", "Skalar*Punkt"},
        /* TagPointScalar */                    {"pont*skalár", "point*scalar", "Punkt*Skalar"},
        /* TagKMTimesG */                       {"(k*m)*G", "(k*m)*G", "(k*m)*G"},
        /* TagRangeCountTargetStart */          {"Range-Count Cél/Start", "Range-Count Target/Start", "Range-Count Ziel/Start"},
        /* TagPointDivScalar */                 {"pont/skalár", "point/scalar", "Punkt/Skalar"},
        /* TagABDivG */                         {"(a/b)*G", "(a/b)*G", "(a/b)*G"},
        /* TagP1PlusNegP2 */                    {"P1+(-P2)", "P1+(-P2)", "P1+(-P2)"},
        /* TagP1PlusP2 */                       {"P1+P2", "P1+P2", "P1+P2"},

        /* BtcEmptyPrivKey */                   {"Üres privát kulcs", "Empty private key", "Leerer privater Schlüssel"},
        /* BtcNegativePrivKey */                {"Negatív érték nem lehet privát kulcs", "A negative value cannot be a private key", "Ein negativer Wert kann kein privater Schlüssel sein"},
        /* BtcEmptyHexKey */                    {"Üres hex kulcs", "Empty hex key", "Leerer Hex-Schlüssel"},
        /* BtcInvalidHexDigitInKey */           {"Érvénytelen hex jegy a kulcsban", "Invalid hex digit in the key", "Ungültige Hex-Ziffer im Schlüssel"},
        /* BtcKeyLargerThan256 */               {"A kulcs nagyobb mint 256 bit", "The key is larger than 256 bits", "Der Schlüssel ist größer als 256 Bit"},
        /* BtcInvalidDecimalKey */              {"Érvénytelen decimális kulcs", "Invalid decimal key", "Ungültiger dezimaler Schlüssel"},
        /* BtcPrivKeyZero */                    {"A privát kulcs nem lehet 0", "The private key cannot be 0", "Der private Schlüssel darf nicht 0 sein"},
        /* BtcPrivKeyGERange */                 {"A privát kulcs >= görbe rend (N)", "The private key >= curve order (N)", "Der private Schlüssel >= Kurvenordnung (N)"},
        /* BtcPubkeyInfinityInvalidScalar */    {"Pubkey infinity — érvénytelen skalár", "Pubkey is infinity — invalid scalar", "Pubkey ist infinity — ungültiger Skalar"},
        /* BtcCompressedPubNeeds66Hex */        {"Compressed pub 66 hex (02/03...) kell.", "Compressed pubkey needs 66 hex chars (02/03...).", "Komprimierter Pubkey benötigt 66 Hex-Zeichen (02/03...)."},

        /* SecpHexCoordOver256 */               {"Hex koordináta > 256 bit", "Hex coordinate > 256 bits", "Hex-Koordinate > 256 Bit"},
        /* SecpParityInfinity */                {"végtelen (infinity)", "infinity", "Unendlich (infinity)"},
        /* SecpParityOdd */                     {"páratlan (odd / 0x03)", "odd (0x03)", "ungerade (0x03)"},
        /* SecpParityEven */                    {"páros (even / 0x02)", "even (0x02)", "gerade (0x02)"},
        /* SecpEmptyHexCoord */                 {"Üres hex koordináta", "Empty hex coordinate", "Leere Hex-Koordinate"},
        /* SecpInvalidHexDigit */               {"Érvénytelen hex jegy", "Invalid hex digit", "Ungültige Hex-Ziffer"},
        /* SecpCoordOver256 */                  {"Koordináta > 256 bit", "Coordinate > 256 bits", "Koordinate > 256 Bit"},
        /* SecpInvNoInverseP */                 {"0-nak nincs inverze", "0 has no inverse", "0 hat keine Inverse"},
        /* SecpInvModPFailed */                 {"Moduláris inverz sikertelen", "Modular inverse failed", "Modulare Inverse fehlgeschlagen"},
        /* SecpInvNoInverseN */                 {"0-nak nincs inverze mod n", "0 has no inverse mod n", "0 hat keine Inverse mod n"},
        /* SecpInvModNFailed */                 {"Moduláris inverz (mod n) sikertelen", "Modular inverse (mod n) failed", "Modulare Inverse (mod n) fehlgeschlagen"},
        /* RangeCountStartInfinity */           {"Range-Count: a Start pont infinity — nincs értelme.", "Range-Count: the Start point is infinity — meaningless.", "Range-Count: der Startpunkt ist infinity — ergibt keinen Sinn."},
        /* RangeCountTargetInfinity */          {"Range-Count: a Cél pont infinity — nincs értelme.", "Range-Count: the Target point is infinity — meaningless.", "Range-Count: der Zielpunkt ist infinity — ergibt keinen Sinn."},
        /* RangeCountFoundZeroSteps */          {"[Szinkronizáció]: A célpont sikeresen beazonosítva. Távolság a startponttól: 0 lépés.", "[Sync]: Target point successfully identified. Distance from the start point: 0 steps.", "[Synchronisation]: Zielpunkt erfolgreich identifiziert. Abstand vom Startpunkt: 0 Schritte."},
        /* RangeCountFoundStepsPrefix */        {"[Szinkronizáció]: A célpont sikeresen beazonosítva. Távolság a startponttól: ", "[Sync]: Target point successfully identified. Distance from the start point: ", "[Synchronisation]: Zielpunkt erfolgreich identifiziert. Abstand vom Startpunkt: "},
        /* RangeCountFoundStepsSuffix */        {" lépés.", " steps.", " Schritte."},
        /* RangeCountNotFoundPrefix */          {"Range-Count: a cél nem található ", "Range-Count: target not found within ", "Range-Count: Ziel nicht gefunden innerhalb von "},
        /* RangeCountNotFoundSuffix */          {" G-lépésen belül (Start + k*G = Cél). Növeld a limitet, vagy kisebb tartomány kell.", " G-steps (Start + k*G = Target). Increase the limit, or use a smaller range.", " G-Schritten (Start + k*G = Ziel). Erhöhe das Limit oder verwende einen kleineren Bereich."},
        /* CompressedPubLength66 */             {"Compressed pubkey hossza 66 hex karakter legyen (02/03 + 64).", "Compressed pubkey must be 66 hex characters (02/03 + 64).", "Komprimierter Pubkey muss 66 Hex-Zeichen haben (02/03 + 64)."},
        /* CompressedPrefix0203 */              {"Compressed prefix csak 02 (even) vagy 03 (odd) lehet.", "Compressed prefix can only be 02 (even) or 03 (odd).", "Komprimiertes Präfix darf nur 02 (gerade) oder 03 (ungerade) sein."},
        /* XCoordNotInField */                  {"X koordináta nincs a mező tartományában.", "X coordinate is outside the field range.", "X-Koordinate liegt außerhalb des Körperbereichs."},
        /* NoSquareRootNotOnCurve */             {"Nem létezik négyzetgyök: az X nincs a görbén (y^2 != x^3+7).", "No square root exists: X is not on the curve (y^2 != x^3+7).", "Keine Quadratwurzel vorhanden: X liegt nicht auf der Kurve (y^2 != x^3+7)."},
        /* UncompressedNotOnCurve */             {"Uncompressed pubkey nincs a görbén (y^2 != x^3+7).", "Uncompressed pubkey is not on the curve (y^2 != x^3+7).", "Unkomprimierter Pubkey liegt nicht auf der Kurve (y^2 != x^3+7)."},
        /* UnknownPubkeyForm */                 {"Ismeretlen pubkey forma: compressed (66) vagy uncompressed (130, 04...) kell.", "Unknown pubkey form: needs compressed (66) or uncompressed (130, 04...).", "Unbekannte Pubkey-Form: benötigt komprimiert (66) oder unkomprimiert (130, 04...)."},

        /* WinTitle */                          {"BigCalc — nagy pontosságú kalkulátor", "BigCalc — high-precision calculator", "BigCalc — hochpräziser Rechner"},
        /* LblLeft */                           {"Bal: egyszám=DEC | 0x..=HEX/skalár | 02/03/04/G = G/görbe", "Left: number=DEC | 0x..=HEX/scalar | 02/03/04/G=curve", "Links: Zahl=DEZ | 0x..=HEX/Skalar | 02/03/04/G=Kurve"},
        /* LblOp */                             {"Művelet", "Operator", "Operator"},
        /* LblRight */                          {"Jobb oldali operandus", "Right-hand operand", "Rechter Operand"},
        /* LblPrec */                           {"Pontosság", "Precision", "Genauigkeit"},
        /* LblExpr */                           {"Teljes kifejezés (opcionális, pl. 0xFF * 1.25)", "Full expression (optional, e.g. 0xFF * 1.25)", "Vollständiger Ausdruck (optional, z. B. 0xFF * 1.25)"},
        /* LblDec */                            {"Eredmény — decimális", "Result — decimal", "Ergebnis — dezimal"},
        /* LblHex */                            {"Eredmény — hexadecimális", "Result — hexadecimal", "Ergebnis — hexadezimal"},
        /* LblHist */                           {"Előzmények — kattintás: kijelölés, dupla katt / Ctrl+C: cella másolása, jobb klikk: menü", "History — click: select, double-click / Ctrl+C: copy cell, right-click: menu", "Verlauf — Klick: auswählen, Doppelklick / Strg+C: Zelle kopieren, Rechtsklick: Menü"},
        /* BtnCalc */                           {"Számítás", "Calculate", "Berechnen"},
        /* BtnClear */                          {"Törlés", "Clear", "Löschen"},
        /* StatusReady */                       {"Készen áll. A három alsó panelt a köztes sávokkal átméretezheted. Számítás gomb: számol.", "Ready. Drag the bars between the three lower panels to resize them. Calculate button: computes.", "Bereit. Die drei unteren Bereiche lassen sich über die Trennleisten in der Größe anpassen. Berechnen-Schaltfläche: berechnet."},
        /* ColExpr */                           {"Kifejezés", "Expression", "Ausdruck"},
        /* ColResult */                         {"Eredmény", "Result", "Ergebnis"},
        /* ColWif */                            {"WIF", "WIF", "WIF"},
        /* ColPub */                            {"Pub (compressed)", "Pub (compressed)", "Pub (komprimiert)"},
        /* ColAddress */                        {"Bitcoin cím", "Bitcoin address", "Bitcoin-Adresse"},
        /* ColCell */                           {"Cella", "Cell", "Zelle"},
        /* CtxCopyCellPrefix */                 {"Másolás: ", "Copy: ", "Kopieren: "},
        /* CtxCopyRow */                        {"Teljes sor másolása", "Copy entire row", "Ganze Zeile kopieren"},
        /* StatusErrorPrefix */                 {"Hiba: ", "Error: ", "Fehler: "},
        /* MsgBoxTitle */                       {"BigCalc", "BigCalc", "BigCalc"},
        /* StatusModePrefix */                  {"Mód: ", "Mode: ", "Modus: "},
        /* StatusAddressSuffix */               {" | Cím: ", " | Address: ", " | Adresse: "},
        /* StatusNoCopyableData */              {"Nincs másolható adat ebben a cellában.", "No copyable data in this cell.", "In dieser Zelle gibt es keine kopierbaren Daten."},
        /* StatusCopiedPrefix */                {"Másolva (", "Copied (", "Kopiert ("},
        /* StatusCopiedSuffix */                {"): ", "): ", "): "},
        /* StatusCopyFailed */                  {"Nem sikerült a vágólapra másolni.", "Could not copy to the clipboard.", "Kopieren in die Zwischenablage fehlgeschlagen."},
        /* StatusFullRowCopied */               {"Teljes sor másolva (tab-okkal elválasztva).", "Entire row copied (tab-separated).", "Ganze Zeile kopiert (tabgetrennt)."},
        /* StatusFieldsCleared */               {"Mezők törölve.", "Fields cleared.", "Felder gelöscht."},
        /* StatusSelectedPrefix */              {"Kijelölve: ", "Selected: ", "Ausgewählt: "},
        /* StatusSelectedSuffix */              {"  (Ctrl+C vagy dupla katt = másolás)", "  (Ctrl+C or double-click = copy)", "  (Strg+C oder Doppelklick = kopieren)"},
        /* LangLabel */                         {"Nyelv", "Language", "Sprache"},
    }};
    return t;
}

inline std::string T(Msg m) {
    const Tri& tri = msgTable()[static_cast<size_t>(m)];
    switch (g_lang) {
    case Lang::EN: return tri.en;
    case Lang::DE: return tri.de;
    default:       return tri.hu;
    }
}

// bigcalc_lang.ini az exe mellett: egyetlen sor "HU"/"EN"/"DE". Nincs -> HU.
inline const wchar_t* langIniName() { return L"bigcalc_lang.ini"; }

inline Lang langFromCode(const std::string& code) {
    if (code == "EN") return Lang::EN;
    if (code == "DE") return Lang::DE;
    return Lang::HU;
}

inline const char* langToCode(Lang l) {
    switch (l) {
    case Lang::EN: return "EN";
    case Lang::DE: return "DE";
    default:       return "HU";
    }
}
