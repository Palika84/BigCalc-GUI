#pragma once
#include "BigInt.hpp"
#include "Lang.hpp"
#include <cctype>
#include <sstream>
#include <string>

struct CalcResult {
    bool ok = false;
    std::string decimal;
    std::string hex;
    std::string intDecimal; // egesz resz (Bitcoin privkey forras)
    std::string error;
    std::string expression;
};

inline std::string trim(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

// bigcalc2.ps1: Parse-AnyFloat — minden bemenet 10-es fixpontos BigInt lesz.
inline BigInt parseAnyFloat(const std::string& raw, int targetPrec) {
    std::string s = trim(raw);
    if (s.empty()) throw std::runtime_error(T(Msg::CalcEmptyNumber));

    // HEX: 0xINT[.FRAC]
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        std::string body = s.substr(2);
        size_t dot = body.find('.');
        std::string intHex = (dot == std::string::npos) ? body : body.substr(0, dot);
        std::string fracHex = (dot == std::string::npos) ? "" : body.substr(dot + 1);
        if (intHex.empty()) throw std::runtime_error(T(Msg::CalcInvalidHexFormat));

        BigInt intVal = BigInt::parseHex(intHex);
        BigInt finalVal = intVal * BigInt::pow10(targetPrec);

        for (size_t i = 0; i < fracHex.size(); ++i) {
            int digit = 0;
            char c = fracHex[i];
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            else throw std::runtime_error(T(Msg::CalcInvalidHexDigit));

            BigInt num = BigInt(digit) * BigInt::pow10(targetPrec);
            BigInt denom = BigInt::one();
            for (size_t j = 0; j <= i; ++j) denom = denom * BigInt(16);
            finalVal = finalVal + (num / denom);
        }
        return finalVal;
    }

    // DEC: [-]INT[.FRAC]
    bool neg = false;
    size_t i = 0;
    if (s[i] == '+' || s[i] == '-') {
        neg = s[i] == '-';
        ++i;
    }
    size_t start = i;
    size_t dot = std::string::npos;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (dot != std::string::npos) throw std::runtime_error(T(Msg::CalcInvalidDecimalFormat));
            dot = i;
        } else if (s[i] < '0' || s[i] > '9') {
            throw std::runtime_error(T(Msg::CalcInvalidDecimalFormat));
        }
    }
    if (start >= s.size() || (dot != std::string::npos && start == dot && dot + 1 >= s.size()))
        throw std::runtime_error(T(Msg::CalcInvalidDecimalFormat));

    std::string intPart = (dot == std::string::npos) ? s.substr(start) : s.substr(start, dot - start);
    std::string fracPart = (dot == std::string::npos) ? "" : s.substr(dot + 1);
    if (intPart.empty()) intPart = "0";

    BigInt base = BigInt::parseDecimal(intPart) * BigInt::pow10(targetPrec);
    BigInt frac = BigInt::zero();
    for (int k = 0; k < targetPrec; ++k) {
        int digit = 0;
        if (k < static_cast<int>(fracPart.size())) digit = fracPart[k] - '0';
        frac = frac + BigInt(digit) * BigInt::pow10(targetPrec - 1 - k);
    }
    BigInt val = base + frac;
    if (neg && !val.isZero()) val = -val;
    return val;
}

inline std::string formatAsDecimal(const BigInt& val, int prec) {
    BigInt scale = BigInt::pow10(prec);
    std::string sign = val.isNegative() ? "-" : "";
    BigInt a = val.abs();
    BigInt intPart = a / scale;
    BigInt rem = a % scale;
    std::string remStr = rem.toDecimal();
    while (static_cast<int>(remStr.size()) < prec) remStr = "0" + remStr;
    if (prec <= 0) return sign + intPart.toDecimal();
    return sign + intPart.toDecimal() + "." + remStr;
}

inline std::string formatAsHex(const BigInt& val, int prec) {
    BigInt scale = BigInt::pow10(prec);
    std::string sign = val.isNegative() ? "-" : "";
    BigInt a = val.abs();
    BigInt intPart = a / scale;
    BigInt rem = a % scale;

    std::string intHex = intPart.toHex();
    std::string fracHex;
    for (int i = 0; i < prec; ++i) {
        rem = rem * BigInt(16);
        BigInt digit = rem / scale;
        rem = rem % scale;
        std::string d = digit.toHex();
        fracHex += d.empty() ? "0" : d;
    }
    if (prec <= 0) return sign + "0x" + intHex;
    return sign + "0x" + intHex + "." + fracHex;
}

// Kifejezés: LEFT OP RIGHT  (OP: + - * /)
// A regex (.+)([+\-*/])(.+) mohó bal oldal miatt az utolsó operátort választjuk,
// kivéve ha a jobb oldal előjelének tűnik — egyszerűen: első operátor a számok között.
inline bool splitExpression(const std::string& expr, std::string& left, char& op, std::string& right) {
    std::string s = trim(expr);
    bool found = false;
    // Az utolsó bináris operátort tartjuk; a jobb oldal unary előjelét átugorjuk.
    for (size_t i = 1; i < s.size(); ++i) {
        char c = s[i];
        if (c != '+' && c != '-' && c != '*' && c != '/') continue;
        char prev = s[i - 1];
        if ((c == '+' || c == '-') && (prev == '+' || prev == '-' || prev == '*' || prev == '/'))
            continue; // unary előjel a jobb oldalon
        std::string L = trim(s.substr(0, i));
        std::string R = trim(s.substr(i + 1));
        if (!L.empty() && !R.empty()) {
            left = L;
            op = c;
            right = R;
            found = true;
        }
    }
    return found;
}

inline CalcResult evaluate(const std::string& expression, int precision) {
    CalcResult out;
    out.expression = trim(expression);
    try {
        if (precision < 0) throw std::runtime_error(T(Msg::CalcPrecisionNegative));
        if (precision > 64) throw std::runtime_error(T(Msg::CalcPrecisionMax));

        std::string left, right;
        char op = 0;
        if (!splitExpression(out.expression, left, op, right))
            throw std::runtime_error(T(Msg::CalcParseExpressionFailed));

        BigInt A = parseAnyFloat(left, precision);
        BigInt B = parseAnyFloat(right, precision);
        BigInt scale = BigInt::pow10(precision);
        BigInt res;

        switch (op) {
        case '+': res = A + B; break;
        case '-': res = A - B; break;
        case '*': res = (A * B) / scale; break;
        case '/':
            if (B.isZero()) throw std::runtime_error(T(Msg::CalcDivisionByZero));
            res = (A * scale) / B;
            break;
        default:
            throw std::runtime_error(T(Msg::CalcUnknownOperator));
        }

        out.decimal = formatAsDecimal(res, precision);
        out.hex = formatAsHex(res, precision);
        {
            BigInt intPart = res.abs() / scale;
            out.intDecimal = (res.isNegative() ? "-" : "") + intPart.toDecimal();
        }
        out.ok = true;
    } catch (const std::exception& ex) {
        out.ok = false;
        out.error = ex.what();
    }
    return out;
}

inline CalcResult evaluateParts(const std::string& left, char op, const std::string& right, int precision) {
    std::ostringstream oss;
    oss << trim(left) << ' ' << op << ' ' << trim(right);
    return evaluate(oss.str(), precision);
}
