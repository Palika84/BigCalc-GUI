#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "Lang.hpp"

// Előjelzett, tetszőleges pontosságú egész (System.Numerics.BigInteger jellegű).
class BigInt {
public:
    BigInt() : neg_(false) { d_ = {0}; }

    BigInt(long long v) : neg_(v < 0) {
        unsigned long long u = v < 0 ? static_cast<unsigned long long>(-v) : static_cast<unsigned long long>(v);
        d_.clear();
        if (u == 0) {
            d_.push_back(0);
            neg_ = false;
            return;
        }
        while (u) {
            d_.push_back(static_cast<uint32_t>(u & 0xFFFFFFFFu));
            u >>= 32;
        }
    }

    static BigInt zero() { return BigInt(0); }
    static BigInt one() { return BigInt(1); }

    bool isZero() const { return d_.size() == 1 && d_[0] == 0; }
    bool isNegative() const { return neg_ && !isZero(); }

    BigInt abs() const {
        BigInt r = *this;
        r.neg_ = false;
        return r;
    }

    int cmpAbs(const BigInt& o) const {
        if (d_.size() != o.d_.size()) return d_.size() < o.d_.size() ? -1 : 1;
        for (size_t i = d_.size(); i-- > 0;) {
            if (d_[i] < o.d_[i]) return -1;
            if (d_[i] > o.d_[i]) return 1;
        }
        return 0;
    }

    int cmp(const BigInt& o) const {
        if (isZero() && o.isZero()) return 0;
        if (neg_ != o.neg_) return neg_ ? -1 : 1;
        int c = cmpAbs(o);
        return neg_ ? -c : c;
    }

    bool operator==(const BigInt& o) const { return cmp(o) == 0; }
    bool operator!=(const BigInt& o) const { return cmp(o) != 0; }
    bool operator<(const BigInt& o) const { return cmp(o) < 0; }
    bool operator<=(const BigInt& o) const { return cmp(o) <= 0; }
    bool operator>(const BigInt& o) const { return cmp(o) > 0; }
    bool operator>=(const BigInt& o) const { return cmp(o) >= 0; }

    BigInt operator-() const {
        BigInt r = *this;
        if (!r.isZero()) r.neg_ = !r.neg_;
        return r;
    }

    BigInt operator+(const BigInt& o) const {
        if (neg_ == o.neg_) {
            BigInt r = addAbs(o);
            r.neg_ = neg_;
            return r;
        }
        int c = cmpAbs(o);
        if (c == 0) return zero();
        if (c > 0) {
            BigInt r = subAbs(o);
            r.neg_ = neg_;
            return r;
        }
        BigInt r = o.subAbs(*this);
        r.neg_ = o.neg_;
        return r;
    }

    BigInt operator-(const BigInt& o) const { return *this + (-o); }

    BigInt operator*(const BigInt& o) const {
        if (isZero() || o.isZero()) return zero();
        BigInt r;
        r.d_.assign(d_.size() + o.d_.size(), 0);
        for (size_t i = 0; i < d_.size(); ++i) {
            uint64_t carry = 0;
            for (size_t j = 0; j < o.d_.size(); ++j) {
                uint64_t cur = r.d_[i + j] + static_cast<uint64_t>(d_[i]) * o.d_[j] + carry;
                r.d_[i + j] = static_cast<uint32_t>(cur);
                carry = cur >> 32;
            }
            size_t k = i + o.d_.size();
            while (carry) {
                uint64_t cur = r.d_[k] + carry;
                r.d_[k] = static_cast<uint32_t>(cur);
                carry = cur >> 32;
                ++k;
            }
        }
        r.neg_ = neg_ != o.neg_;
        r.trim();
        return r;
    }

    // Egész osztás (mint C# BigInteger: a nullához közelít negatívnál is truncating).
    BigInt operator/(const BigInt& o) const {
        if (o.isZero()) throw std::runtime_error(T(Msg::BigIntDivisionByZero));
        BigInt q, r;
        divModAbs(*this, o, q, r);
        q.neg_ = neg_ != o.neg_ && !q.isZero();
        return q;
    }

    BigInt operator%(const BigInt& o) const {
        if (o.isZero()) throw std::runtime_error(T(Msg::BigIntDivisionByZero));
        BigInt q, r;
        divModAbs(*this, o, q, r);
        r.neg_ = neg_ && !r.isZero();
        return r;
    }

    static BigInt pow10(int exp) {
        if (exp < 0) throw std::runtime_error(T(Msg::BigIntNegativeExponent));
        BigInt r(1);
        BigInt ten(10);
        for (int i = 0; i < exp; ++i) r = r * ten;
        return r;
    }

    static BigInt parseDecimal(const std::string& s) {
        if (s.empty()) throw std::runtime_error(T(Msg::BigIntEmptyNumber));
        size_t i = 0;
        bool neg = false;
        if (s[i] == '+' || s[i] == '-') {
            neg = s[i] == '-';
            ++i;
        }
        if (i >= s.size()) throw std::runtime_error(T(Msg::BigIntInvalidDecimal));
        BigInt r;
        BigInt ten(10);
        for (; i < s.size(); ++i) {
            if (s[i] < '0' || s[i] > '9') throw std::runtime_error(T(Msg::BigIntInvalidDecimalDigit));
            r = r * ten + BigInt(s[i] - '0');
        }
        r.neg_ = neg && !r.isZero();
        return r;
    }

    static BigInt parseHex(const std::string& hexDigits) {
        BigInt r;
        for (char c : hexDigits) {
            int digit = hexValue(c);
            if (digit < 0) throw std::runtime_error(T(Msg::BigIntInvalidHexDigit));
            r = r.mulSmall(16) + BigInt(digit);
        }
        return r;
    }

    std::string toDecimal() const {
        if (isZero()) return "0";
        BigInt rem = abs();
        std::string s;
        BigInt ten(10);
        while (!rem.isZero()) {
            BigInt q, r;
            divModAbs(rem, ten, q, r);
            s.push_back(static_cast<char>('0' + r.d_[0]));
            rem = q;
        }
        if (neg_) s.push_back('-');
        std::reverse(s.begin(), s.end());
        return s;
    }

    std::string toHex() const {
        if (isZero()) return "0";
        BigInt rem = abs();
        std::string s;
        while (!rem.isZero()) {
            uint32_t nibble = rem.d_[0] & 0xF;
            s.push_back(static_cast<char>(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10)));
            rem = rem.shr4();
        }
        if (neg_) s.push_back('-');
        std::reverse(s.begin(), s.end());
        return s;
    }

private:
    std::vector<uint32_t> d_; // little-endian limbs
    bool neg_;

    void trim() {
        while (d_.size() > 1 && d_.back() == 0) d_.pop_back();
        if (isZero()) neg_ = false;
    }

    BigInt mulSmall(uint32_t m) const {
        BigInt r;
        r.d_.assign(d_.size() + 1, 0);
        uint64_t carry = 0;
        for (size_t i = 0; i < d_.size(); ++i) {
            uint64_t cur = static_cast<uint64_t>(d_[i]) * m + carry;
            r.d_[i] = static_cast<uint32_t>(cur);
            carry = cur >> 32;
        }
        r.d_[d_.size()] = static_cast<uint32_t>(carry);
        r.neg_ = neg_;
        r.trim();
        return r;
    }

    BigInt shr4() const {
        BigInt r;
        r.d_.resize(d_.size());
        uint32_t carry = 0;
        for (size_t i = d_.size(); i-- > 0;) {
            uint32_t cur = d_[i];
            r.d_[i] = (cur >> 4) | (carry << 28);
            carry = cur & 0xF;
        }
        r.neg_ = neg_;
        r.trim();
        return r;
    }

    BigInt addAbs(const BigInt& o) const {
        BigInt r;
        size_t n = (std::max)(d_.size(), o.d_.size());
        r.d_.assign(n + 1, 0);
        uint64_t carry = 0;
        for (size_t i = 0; i < n; ++i) {
            uint64_t a = i < d_.size() ? d_[i] : 0;
            uint64_t b = i < o.d_.size() ? o.d_[i] : 0;
            uint64_t sum = a + b + carry;
            r.d_[i] = static_cast<uint32_t>(sum);
            carry = sum >> 32;
        }
        r.d_[n] = static_cast<uint32_t>(carry);
        r.trim();
        return r;
    }

    BigInt subAbs(const BigInt& o) const {
        // |*this| >= |o|
        BigInt r;
        r.d_.assign(d_.size(), 0);
        int64_t borrow = 0;
        for (size_t i = 0; i < d_.size(); ++i) {
            int64_t a = d_[i];
            int64_t b = i < o.d_.size() ? o.d_[i] : 0;
            int64_t diff = a - b - borrow;
            if (diff < 0) {
                diff += (1LL << 32);
                borrow = 1;
            } else {
                borrow = 0;
            }
            r.d_[i] = static_cast<uint32_t>(diff);
        }
        r.trim();
        return r;
    }

    static void divModAbs(const BigInt& aIn, const BigInt& bIn, BigInt& q, BigInt& r) {
        BigInt a = aIn.abs();
        BigInt b = bIn.abs();
        if (a.cmpAbs(b) < 0) {
            q = zero();
            r = a;
            return;
        }
        if (b.d_.size() == 1) {
            divModSmall(a, b.d_[0], q, r);
            return;
        }

        // Binary long division
        q = zero();
        r = zero();
        for (int i = static_cast<int>(a.bitLength()) - 1; i >= 0; --i) {
            r = r.mulSmall(2);
            if (a.bit(i)) r.d_[0] |= 1u;
            r.trim();
            if (r.cmpAbs(b) >= 0) {
                r = r.subAbs(b);
                q = q.setBit(i);
            }
        }
        q.trim();
        r.trim();
    }

    static void divModSmall(const BigInt& a, uint32_t b, BigInt& q, BigInt& r) {
        q.d_.assign(a.d_.size(), 0);
        uint64_t rem = 0;
        for (size_t i = a.d_.size(); i-- > 0;) {
            uint64_t cur = (rem << 32) | a.d_[i];
            q.d_[i] = static_cast<uint32_t>(cur / b);
            rem = cur % b;
        }
        q.neg_ = false;
        q.trim();
        r = BigInt(static_cast<long long>(rem));
    }

    size_t bitLength() const {
        if (isZero()) return 0;
        size_t bits = (d_.size() - 1) * 32;
        uint32_t top = d_.back();
        while (top) {
            ++bits;
            top >>= 1;
        }
        return bits;
    }

    bool bit(int i) const {
        if (i < 0) return false;
        size_t limb = static_cast<size_t>(i) / 32;
        size_t off = static_cast<size_t>(i) % 32;
        if (limb >= d_.size()) return false;
        return ((d_[limb] >> off) & 1u) != 0;
    }

    BigInt setBit(int i) const {
        BigInt r = *this;
        size_t limb = static_cast<size_t>(i) / 32;
        size_t off = static_cast<size_t>(i) % 32;
        if (limb >= r.d_.size()) r.d_.resize(limb + 1, 0);
        r.d_[limb] |= (1u << off);
        return r;
    }

    static int hexValue(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }
};
