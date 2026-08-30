#pragma once

#include "Calculator.hpp"
#include "Lang.hpp"
#include "crypto/BitcoinCrypto.hpp"
#include "crypto/SecpAffine.hpp"

#include <cctype>
#include <string>

// Automatikus mod a bemenet formaja alapjan:
//  - kerek egesz / tizedes  -> DEC skalár
//  - 0x...                  -> HEX/SKALAR
//  - 02/03/04... vagy "G"   -> G/GORBE (pont)
//
// G-modban a skalár k ertek = k*G pont (pl. 0x2 == G+G).

enum class OperandKind {
    Decimal,     // 10-es szam (egesz vagy tort)
    HexScalar,   // 0x...
    CurvePoint,  // 02/03 compressed, 04 uncompressed, vagy "G"
    Invalid
};

struct UnifiedResult {
    bool ok = false;
    std::string mode;          // "DEC" | "HEX/SKALAR" | "G/GORBE" | "G/RANGE-COUNT"
    std::string expression;
    std::string decimal;       // skalár eredmeny VAGY G-mod leiras
    std::string hex;           // skalár hex VAGY compressed pont
    std::string intDecimal;    // skalár egesz (BTC WIF forras); G-modban ures
    std::string pointCompressed;
    std::string pointX;
    std::string pointY;
    bool isPointResult = false;
    bool isRangeCount = false; // Overshoot & Range-Count (pont/pont /)
    std::string syncMessage;   // szinkron hid / naplo
    std::string error;
};

inline std::string extractHexDigits(const std::string& raw) {
    std::string hex;
    for (char c : raw) {
        if (std::isxdigit(static_cast<unsigned char>(c)))
            hex.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return hex;
}

inline OperandKind classifyOperand(const std::string& raw) {
    std::string s = trim(raw);
    if (s.empty()) return OperandKind::Invalid;

    if (s.size() == 1 && (s[0] == 'G' || s[0] == 'g'))
        return OperandKind::CurvePoint;

    std::string hex = extractHexDigits(s);
    // Pubkey: 02/03 + 64, vagy 04 + 128
    if (hex.size() >= 66 && hex[0] == '0' && (hex[1] == '2' || hex[1] == '3'))
        return OperandKind::CurvePoint;
    if (hex.size() >= 130 && hex[0] == '0' && hex[1] == '4')
        return OperandKind::CurvePoint;

    // 0x... skalár (NEM pubkey — a pubkeynek nincs 0x prefix igenye, de ha 0x02...66 hex, az is pont)
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        std::string body = s.substr(2);
        std::string h = extractHexDigits(body);
        if (h.size() >= 66 && h[0] == '0' && (h[1] == '2' || h[1] == '3'))
            return OperandKind::CurvePoint;
        if (h.size() >= 130 && h[0] == '0' && h[1] == '4')
            return OperandKind::CurvePoint;
        return OperandKind::HexScalar;
    }

    // Decimalis (opcionalis elojeel, opcionalis tizedes)
    size_t i = 0;
    if (s[i] == '+' || s[i] == '-') ++i;
    bool digit = false, dot = false;
    for (; i < s.size(); ++i) {
        if (s[i] >= '0' && s[i] <= '9') {
            digit = true;
        } else if (s[i] == '.' && !dot) {
            dot = true;
        } else {
            return OperandKind::Invalid;
        }
    }
    return digit ? OperandKind::Decimal : OperandKind::Invalid;
}

inline bool parseScalarInteger(const std::string& raw, BigInt& out, std::string& err) {
    try {
        std::string s = trim(raw);
        if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            out = BigInt::parseHex(extractHexDigits(s.substr(2)));
            return true;
        }
        // csak egesz resz (G-modhoz)
        size_t dot = s.find('.');
        std::string ip = (dot == std::string::npos) ? s : s.substr(0, dot);
        if (ip.empty() || ip == "+" || ip == "-") ip = "0";
        out = BigInt::parseDecimal(ip);
        return true;
    } catch (const std::exception& ex) {
        err = ex.what();
        return false;
    }
}

inline bool resolveToPoint(const std::string& raw, OperandKind kind, SecpPoint& out, std::string& err) {
    SecpAffine secp;
    std::string s = trim(raw);

    if (s.size() == 1 && (s[0] == 'G' || s[0] == 'g')) {
        out = secp.generator();
        return true;
    }

    if (kind == OperandKind::CurvePoint) {
        // 0x prefix levagasa ha van
        if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            s = s.substr(2);
        return secp.parsePubkeyAny(s, out, err);
    }

    if (kind == OperandKind::Decimal || kind == OperandKind::HexScalar) {
        // Skalár k -> pont k*G   (0x2 == G+G)
        BigInt k;
        if (!parseScalarInteger(raw, k, err)) return false;
        if (k.isZero() || k.isNegative()) {
            err = T(Msg::ErrGModScalarPositive);
            return false;
        }
        out = secp.scalarMulBase(k);
        if (out.infinity) {
            err = T(Msg::ErrGModInfinity);
            return false;
        }
        return true;
    }

    err = T(Msg::ErrUnknownOperandG);
    return false;
}

inline std::string kindName(OperandKind k) {
    switch (k) {
    case OperandKind::Decimal: return T(Msg::ModeDec);
    case OperandKind::HexScalar: return T(Msg::ModeHexScalar);
    case OperandKind::CurvePoint: return T(Msg::ModeGPoint);
    default: return "?";
    }
}

inline void fillPointResult(UnifiedResult& u, const SecpPoint& R, const std::string& note) {
    u.ok = true;
    u.isPointResult = true;
    if (R.infinity) {
        u.decimal = T(Msg::NoteInfinityPoint);
        u.hex = "00";
        u.pointCompressed = "";
        u.pointX = std::string(64, '0');
        u.pointY = std::string(64, '0');
    } else {
        u.pointCompressed = R.compressedHex();
        u.pointX = R.xHex();
        u.pointY = R.yHex();
        u.decimal = note.empty() ? (T(Msg::NoteGPointResultPrefix) + u.pointX) : note;
        u.hex = u.pointCompressed;
    }
}

inline bool isScalarKind(OperandKind k) {
    return k == OperandKind::Decimal || k == OperandKind::HexScalar;
}

inline UnifiedResult evaluateSmart(const std::string& leftRaw, char op, const std::string& rightRaw, int precision) {
    UnifiedResult u;
    u.expression = trim(leftRaw) + " " + op + " " + trim(rightRaw);

    OperandKind kl = classifyOperand(leftRaw);
    OperandKind kr = classifyOperand(rightRaw);
    if (kl == OperandKind::Invalid) {
        u.error = T(Msg::ErrLeftOperandInvalid) + trim(leftRaw);
        return u;
    }
    if (kr == OperandKind::Invalid) {
        u.error = T(Msg::ErrRightOperandInvalid) + trim(rightRaw);
        return u;
    }

    const bool leftIsPointForm = (kl == OperandKind::CurvePoint);
    const bool rightIsPointForm = (kr == OperandKind::CurvePoint);
    const bool useGMode = leftIsPointForm || rightIsPointForm;

    if (!useGMode) {
        // --- Skalár / DEC / HEX kalkulator (meglevo motor) ---
        CalcResult cr = evaluate(u.expression, precision);
        u.ok = cr.ok;
        u.error = cr.error;
        u.decimal = cr.decimal;
        u.hex = cr.hex;
        u.intDecimal = cr.intDecimal;
        u.isPointResult = false;
        if (kl == OperandKind::HexScalar || kr == OperandKind::HexScalar)
            u.mode = T(Msg::ModeHexScalar);
        else
            u.mode = T(Msg::ModeDec);
        return u;
    }

    // --- G / GORBE mod ---
    // +  : P1 + P2
    // -  : P1 - P2 = P1 + (-P2),  -P2 = (x, p-y)
    // *  : k * P  (vagy P * k)
    // /  : P / k = (k^{-1} mod n) * P
    u.mode = T(Msg::ModeGCurve);
    SecpAffine secp;
    std::string err;
    const bool leftPoint = leftIsPointForm;
    const bool rightPoint = rightIsPointForm;
    const bool leftScalar = isScalarKind(kl);
    const bool rightScalar = isScalarKind(kr);

    auto tag = [&](const std::string& extra) {
        u.expression = u.expression + "  [" + kindName(kl) + " " + op + " " + kindName(kr) + " -> G"
                       + (extra.empty() ? "" : ("; " + extra)) + "]";
    };

    // --- * : skalár * pont ---
    if (op == '*') {
        BigInt k;
        SecpPoint P;
        if (leftScalar && rightPoint) {
            if (!parseScalarInteger(leftRaw, k, err) || !resolveToPoint(rightRaw, kr, P, err)) {
                u.error = err;
                return u;
            }
            fillPointResult(u, secp.scalarMul(P, k), T(Msg::NoteKTimesP));
            tag(T(Msg::TagScalarPoint));
            return u;
        }
        if (leftPoint && rightScalar) {
            if (!resolveToPoint(leftRaw, kl, P, err) || !parseScalarInteger(rightRaw, k, err)) {
                u.error = err;
                return u;
            }
            fillPointResult(u, secp.scalarMul(P, k), T(Msg::NotePTimesK));
            tag(T(Msg::TagPointScalar));
            return u;
        }
        if (leftScalar && rightScalar) {
            // mindketto skalár, de G-kontextus (pl. egyik oldalrol jott) — (k*m)*G
            BigInt m;
            if (!parseScalarInteger(leftRaw, k, err) || !parseScalarInteger(rightRaw, m, err)) {
                u.error = err;
                return u;
            }
            fillPointResult(u, secp.scalarMulBase(k * m),
                            T(Msg::NoteGModePrefix) + "(" + trim(leftRaw) + "*" + trim(rightRaw) + ")*G");
            tag(T(Msg::TagKMTimesG));
            return u;
        }
        u.error = T(Msg::ErrGModMulNeedsScalarPoint);
        return u;
    }

    // --- / ---
    //  pont / pont  -> Overshoot & Range-Count (Cel / Start): Start-tol +G, step_count
    //  pont / skalár -> (k^{-1} mod n) * P
    if (op == '/') {
        if (leftPoint && rightPoint) {
            // Bal = Cel pubkey, Jobb = Start/bazis pubkey
            SecpPoint target, start;
            if (!resolveToPoint(leftRaw, kl, target, err)) {
                u.error = T(Msg::ErrRangeCountTargetPrefix) + err;
                return u;
            }
            if (!resolveToPoint(rightRaw, kr, start, err)) {
                u.error = T(Msg::ErrRangeCountStartPrefix) + err;
                return u;
            }
            RangeCountResult rc = secp.rangeCountFromTo(start, target);
            if (!rc.ok) {
                u.error = rc.error;
                return u;
            }
            u.ok = true;
            u.mode = T(Msg::ModeGRangeCount);
            u.isPointResult = false;
            u.isRangeCount = true;
            u.decimal = rc.stepDecimal;
            u.hex = rc.stepHex;
            u.intDecimal.clear(); // tavolsag != privatkulcs
            u.syncMessage = rc.syncMessage;
            tag(T(Msg::TagRangeCountTargetStart));
            return u;
        }
        if (leftScalar && rightPoint) {
            u.error = T(Msg::ErrGModDivScalarPointUndefined);
            return u;
        }
        BigInt k;
        SecpPoint P;
        if (leftPoint && rightScalar) {
            // P / k
            if (!resolveToPoint(leftRaw, kl, P, err) || !parseScalarInteger(rightRaw, k, err)) {
                u.error = err;
                return u;
            }
            if (k.isZero() || k.isNegative()) {
                u.error = T(Msg::ErrGModDivisorPositive);
                return u;
            }
            try {
                fillPointResult(u, secp.scalarDiv(P, k), T(Msg::NotePDivKFormula));
            } catch (const std::exception& ex) {
                u.error = T(Msg::ErrGModSlashGenericPrefix) + ex.what();
                return u;
            }
            tag(T(Msg::TagPointDivScalar));
            return u;
        }
        if (leftScalar && rightScalar) {
            // (a/b)*G = (a * b^{-1} mod n) * G
            BigInt a, b;
            if (!parseScalarInteger(leftRaw, a, err) || !parseScalarInteger(rightRaw, b, err)) {
                u.error = err;
                return u;
            }
            if (b.isZero() || b.isNegative()) {
                u.error = T(Msg::ErrGModDivisorPositive);
                return u;
            }
            try {
                BigInt q = secp.modN(a * secp.invN(b));
                fillPointResult(u, secp.scalarMulBase(q), T(Msg::NoteABDivGFormula));
            } catch (const std::exception& ex) {
                u.error = T(Msg::ErrGModSlashGenericPrefix) + ex.what();
                return u;
            }
            tag(T(Msg::TagABDivG));
            return u;
        }
        u.error = T(Msg::ErrGModDivNeedsRangeCountOrScalar);
        return u;
    }

    // --- + / - : mindket oldalt pontra oldjuk (skalár -> k*G) ---
    if (op != '+' && op != '-') {
        u.error = T(Msg::ErrGModSupportedOps);
        return u;
    }

    SecpPoint A, B;
    if (!resolveToPoint(leftRaw, kl, A, err)) {
        u.error = T(Msg::ErrLeftGPrefix) + err;
        return u;
    }
    if (!resolveToPoint(rightRaw, kr, B, err)) {
        u.error = T(Msg::ErrRightGPrefix) + err;
        return u;
    }

    SecpPoint R = (op == '+') ? secp.add(A, B) : secp.sub(A, B); // sub = add(A, negate(B))
    std::string note = (op == '-')
                           ? (T(Msg::NoteP1PlusNegP2Prefix) + (R.infinity ? std::string(64, '0') : R.xHex()))
                           : (T(Msg::NoteGPointResultPrefix) + (R.infinity ? std::string(64, '0') : R.xHex()));
    if (leftScalar || rightScalar) note += T(Msg::SuffixScalarToG);
    fillPointResult(u, R, note);
    tag(op == '-' ? T(Msg::TagP1PlusNegP2) : T(Msg::TagP1PlusP2));
    return u;
}

inline UnifiedResult evaluateSmartExpression(const std::string& expression, int precision) {
    std::string left, right;
    char op = 0;
    if (!splitExpression(expression, left, op, right)) {
        UnifiedResult u;
        u.error = T(Msg::ErrParseExpressionGeneric);
        return u;
    }
    return evaluateSmart(left, op, right, precision);
}
