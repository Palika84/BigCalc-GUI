#include "SecpAffine.hpp"
#include "../Lang.hpp"

#include <cctype>
#include <stdexcept>
#include <string>

namespace {

std::string strip0x(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s = s.substr(2);
    return s;
}

std::string pad64(std::string hex) {
    for (char& c : hex) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (hex.size() > 64) throw std::runtime_error(T(Msg::SecpHexCoordOver256));
    if (hex.size() < 64) hex = std::string(64 - hex.size(), '0') + hex;
    return hex;
}

} // namespace

SecpAffine::SecpAffine() {
    // p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFC2F
    p_ = BigInt::parseHex("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F");
    // n = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE BAAEDCE6 AF48A03B BFD25E8C D0364141
    n_ = BigInt::parseHex("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
    seven_ = BigInt(7);
    g_.x = BigInt::parseHex("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");
    g_.y = BigInt::parseHex("483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8");
    g_.infinity = false;
}

bool SecpPoint::isOdd() const {
    if (infinity) return false;
    // y odd ha legalso bit 1 — toHex utolso char, vagy % 2
    BigInt two(2);
    return (y % two) == BigInt(1);
}

std::string SecpPoint::xHex() const {
    if (infinity) return std::string(64, '0');
    return pad64(x.toHex());
}

std::string SecpPoint::yHex() const {
    if (infinity) return std::string(64, '0');
    return pad64(y.toHex());
}

std::string SecpPoint::parityLabel() const {
    if (infinity) return T(Msg::SecpParityInfinity);
    return isOdd() ? T(Msg::SecpParityOdd) : T(Msg::SecpParityEven);
}

std::string SecpPoint::compressedHex() const {
    if (infinity) return std::string(66, '0');
    std::string s = isOdd() ? "03" : "02";
    s += xHex();
    return s;
}

bool SecpAffine::parseHexCoord(const std::string& hex, BigInt& out, std::string& err) {
    try {
        std::string s = strip0x(hex);
        if (s.empty()) {
            err = T(Msg::SecpEmptyHexCoord);
            return false;
        }
        for (char c : s) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                err = T(Msg::SecpInvalidHexDigit);
                return false;
            }
        }
        if (s.size() > 64) {
            err = T(Msg::SecpCoordOver256);
            return false;
        }
        out = BigInt::parseHex(s);
        return true;
    } catch (const std::exception& ex) {
        err = ex.what();
        return false;
    }
}

BigInt SecpAffine::modP(const BigInt& a) const {
    BigInt r = a % p_;
    if (r.isNegative()) r = r + p_;
    return r;
}

BigInt SecpAffine::addP(const BigInt& a, const BigInt& b) const {
    return modP(a + b);
}

BigInt SecpAffine::subP(const BigInt& a, const BigInt& b) const {
    return modP(a - b);
}

BigInt SecpAffine::mulP(const BigInt& a, const BigInt& b) const {
    return modP(a * b);
}

BigInt SecpAffine::powP(BigInt base, BigInt exp) const {
    BigInt result(1);
    base = modP(base);
    while (!exp.isZero()) {
        if ((exp % BigInt(2)) == BigInt(1))
            result = mulP(result, base);
        base = mulP(base, base);
        exp = exp / BigInt(2);
    }
    return result;
}

BigInt SecpAffine::invP(const BigInt& a) const {
    // Extended Euclidean: inv(a) mod p
    BigInt t(0), newt(1);
    BigInt r = p_;
    BigInt newr = modP(a);
    if (newr.isZero()) throw std::runtime_error(T(Msg::SecpInvNoInverseP));

    while (!newr.isZero()) {
        BigInt q = r / newr;
        BigInt tmp = newt;
        newt = t - q * newt;
        t = tmp;
        tmp = newr;
        newr = r - q * newr;
        r = tmp;
    }
    if (r != BigInt(1)) throw std::runtime_error(T(Msg::SecpInvModPFailed));
    return modP(t);
}

SecpPoint SecpAffine::generator() const { return g_; }

SecpPoint SecpAffine::negate(const SecpPoint& p) const {
    if (p.infinity) return p;
    // -P = (x, p - y)  a primmezoben
    SecpPoint n = p;
    n.y = subP(p_, p.y);
    return n;
}

SecpPoint SecpAffine::add(const SecpPoint& a, const SecpPoint& b) const {
    if (a.infinity) return b;
    if (b.infinity) return a;

    if (a.x == b.x) {
        // a == -b  =>  infinity  (y_a + y_b ≡ 0 mod p)
        if (addP(a.y, b.y).isZero()) {
            SecpPoint inf;
            inf.infinity = true;
            return inf;
        }
        if (a.y != b.y) {
            SecpPoint inf;
            inf.infinity = true;
            return inf;
        }
        // a == b -> double
        if (a.y.isZero()) {
            SecpPoint inf;
            inf.infinity = true;
            return inf;
        }
        BigInt xx = mulP(a.x, a.x);
        BigInt num = addP(addP(xx, xx), xx);
        BigInt den = addP(a.y, a.y);
        BigInt lam = mulP(num, invP(den));
        BigInt x3 = subP(subP(mulP(lam, lam), a.x), b.x);
        BigInt y3 = subP(mulP(lam, subP(a.x, x3)), a.y);
        SecpPoint r;
        r.x = x3;
        r.y = y3;
        return r;
    }

    BigInt num = subP(b.y, a.y);
    BigInt den = subP(b.x, a.x);
    BigInt lam = mulP(num, invP(den));
    BigInt x3 = subP(subP(mulP(lam, lam), a.x), b.x);
    BigInt y3 = subP(mulP(lam, subP(a.x, x3)), a.y);
    SecpPoint r;
    r.x = x3;
    r.y = y3;
    return r;
}

SecpPoint SecpAffine::sub(const SecpPoint& a, const SecpPoint& b) const {
    // P1 - P2 = P1 + (-P2), ahol -P2 = (x, p - y)
    return add(a, negate(b));
}

SecpPoint SecpAffine::dbl(const SecpPoint& p) const {
    return add(p, p);
}

BigInt SecpAffine::modN(const BigInt& a) const {
    BigInt r = a % n_;
    if (r.isNegative()) r = r + n_;
    return r;
}

BigInt SecpAffine::invN(const BigInt& a) const {
    BigInt t(0), newt(1);
    BigInt r = n_;
    BigInt newr = modN(a);
    if (newr.isZero()) throw std::runtime_error(T(Msg::SecpInvNoInverseN));

    while (!newr.isZero()) {
        BigInt q = r / newr;
        BigInt tmp = newt;
        newt = t - q * newt;
        t = tmp;
        tmp = newr;
        newr = r - q * newr;
        r = tmp;
    }
    if (r != BigInt(1)) throw std::runtime_error(T(Msg::SecpInvModNFailed));
    return modN(t);
}

SecpPoint SecpAffine::scalarMul(const SecpPoint& p, const BigInt& k) const {
    if (p.infinity) return p;
    BigInt kk = modN(k);
    if (kk.isZero()) {
        SecpPoint inf;
        inf.infinity = true;
        return inf;
    }

    std::string hex = kk.toHex();
    for (char& c : hex) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    while (hex.size() < 64) hex = "0" + hex;
    if (hex.size() > 64) hex = hex.substr(hex.size() - 64);

    uint8_t bytes[32];
    for (int i = 0; i < 32; ++i)
        bytes[i] = static_cast<uint8_t>(std::stoul(hex.substr(static_cast<size_t>(i * 2), 2), nullptr, 16));

    SecpPoint r;
    r.infinity = true;
    for (int i = 0; i < 32; ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            if (!r.infinity) r = dbl(r);
            if ((bytes[i] >> bit) & 1u) {
                if (r.infinity) r = p;
                else r = add(r, p);
            }
        }
    }
    return r;
}

SecpPoint SecpAffine::scalarMulBase(const BigInt& k) const {
    return scalarMul(g_, k);
}

SecpPoint SecpAffine::scalarDiv(const SecpPoint& p, const BigInt& k) const {
    // P / k ≡ (k^{-1} mod n) * P
    BigInt inv = invN(k);
    return scalarMul(p, inv);
}

bool SecpAffine::pointsEqual(const SecpPoint& a, const SecpPoint& b) {
    if (a.infinity && b.infinity) return true;
    if (a.infinity || b.infinity) return false;
    return a.x == b.x && a.y == b.y;
}

RangeCountResult SecpAffine::rangeCountFromTo(const SecpPoint& start, const SecpPoint& target,
                                             uint64_t maxSteps) const {
    RangeCountResult out;
    if (start.infinity) {
        out.error = T(Msg::RangeCountStartInfinity);
        return out;
    }
    if (target.infinity) {
        out.error = T(Msg::RangeCountTargetInfinity);
        return out;
    }

    SecpPoint current = start;
    uint64_t step = 0;

    if (pointsEqual(current, target)) {
        out.ok = true;
        out.found = true;
        out.steps = 0;
        out.stepDecimal = "0";
        out.stepHex = "0x0";
        out.syncMessage = T(Msg::RangeCountFoundZeroSteps);
        return out;
    }

    const SecpPoint G = generator();
    while (step < maxSteps) {
        current = add(current, G);
        ++step;
        if (pointsEqual(current, target)) {
            out.ok = true;
            out.found = true;
            out.steps = step;
            out.stepDecimal = std::to_string(step);
            BigInt sc = BigInt::parseDecimal(out.stepDecimal);
            out.stepHex = "0x" + sc.toHex();
            out.syncMessage =
                T(Msg::RangeCountFoundStepsPrefix) + out.stepDecimal + T(Msg::RangeCountFoundStepsSuffix);
            return out;
        }
    }

    out.ok = false;
    out.found = false;
    out.steps = maxSteps;
    out.error =
        T(Msg::RangeCountNotFoundPrefix) + std::to_string(maxSteps) + T(Msg::RangeCountNotFoundSuffix);
    return out;
}

bool SecpAffine::decompressCompressed(const std::string& compressedHex, SecpPoint& out, std::string& err) const {
    try {
        std::string hex;
        for (char c : compressedHex) {
            if (std::isxdigit(static_cast<unsigned char>(c)))
                hex.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        if (hex.size() > 66 && hex[0] == '0' && (hex[1] == '2' || hex[1] == '3'))
            hex = hex.substr(0, 66);
        if (hex.size() != 66) {
            err = T(Msg::CompressedPubLength66);
            return false;
        }
        int prefix = 0;
        if (hex[0] == '0' && hex[1] == '2') prefix = 2;
        else if (hex[0] == '0' && hex[1] == '3') prefix = 3;
        else {
            err = T(Msg::CompressedPrefix0203);
            return false;
        }

        BigInt x = BigInt::parseHex(hex.substr(2));
        if (x >= p_) {
            err = T(Msg::XCoordNotInField);
            return false;
        }

        BigInt x2 = mulP(x, x);
        BigInt x3 = mulP(x2, x);
        BigInt rhs = addP(x3, seven_);
        BigInt exp = (p_ + BigInt(1)) / BigInt(4);
        BigInt y = powP(rhs, exp);
        if (mulP(y, y) != rhs) {
            err = T(Msg::NoSquareRootNotOnCurve);
            return false;
        }

        bool yOdd = (y % BigInt(2)) == BigInt(1);
        bool wantOdd = (prefix == 3);
        if (yOdd != wantOdd)
            y = subP(p_, y);

        out.x = x;
        out.y = y;
        out.infinity = false;
        return true;
    } catch (const std::exception& ex) {
        err = ex.what();
        return false;
    }
}

bool SecpAffine::parsePubkeyAny(const std::string& pubkeyHex, SecpPoint& out, std::string& err) const {
    std::string hex;
    for (char c : pubkeyHex) {
        if (std::isxdigit(static_cast<unsigned char>(c)))
            hex.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    if (hex.size() >= 2 && hex[0] == '0' && hex[1] == '4' && hex.size() >= 130) {
        hex = hex.substr(0, 130);
        try {
            out.x = BigInt::parseHex(hex.substr(2, 64));
            out.y = BigInt::parseHex(hex.substr(66, 64));
            out.infinity = false;
            BigInt y2 = mulP(out.y, out.y);
            BigInt x2 = mulP(out.x, out.x);
            BigInt rhs = addP(mulP(x2, out.x), seven_);
            if (y2 != rhs) {
                err = T(Msg::UncompressedNotOnCurve);
                return false;
            }
            return true;
        } catch (const std::exception& ex) {
            err = ex.what();
            return false;
        }
    }

    if (hex.size() >= 66 && hex[0] == '0' && (hex[1] == '2' || hex[1] == '3'))
        return decompressCompressed(hex.substr(0, 66), out, err);

    err = T(Msg::UnknownPubkeyForm);
    return false;
}
