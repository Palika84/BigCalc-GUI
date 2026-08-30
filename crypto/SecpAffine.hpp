#pragma once

#include "BigInt.hpp"

#include <array>
#include <cstdint>
#include <string>

// Megbizhato affin secp256k1 (BigInt mod P) — a GUI gorbe motorja ehhez kotodik.
struct SecpPoint {
    BigInt x;
    BigInt y;
    bool infinity = false;

    bool isOdd() const;
    std::string xHex() const;
    std::string yHex() const;
    std::string parityLabel() const;
    std::string compressedHex() const;
};

struct RangeCountResult {
    bool ok = false;
    bool found = false;
    std::string error;
    std::string stepDecimal;   // step_count decimalis
    std::string stepHex;       // 0x...
    std::string syncMessage;   // szinkron naplo szoveg
    uint64_t steps = 0;
};

// Overshoot & Range-Count: Start-tol G-t adogatva a Cel-ig; max lepes a GUI vedelmehez.
inline constexpr uint64_t kRangeCountDefaultLimit = 2000000ULL;

class SecpAffine {
public:
    SecpAffine();

    SecpPoint generator() const;
    SecpPoint negate(const SecpPoint& p) const;
    SecpPoint add(const SecpPoint& a, const SecpPoint& b) const;
    // P1 - P2 = P1 + (-P2), ahol -P2 = (x, p - y)
    SecpPoint sub(const SecpPoint& a, const SecpPoint& b) const;
    SecpPoint dbl(const SecpPoint& p) const;
    // k * G  (helyes skalarszorzas a Bitcoin pubkey-hez)
    SecpPoint scalarMulBase(const BigInt& k) const;
    // k * P  (tetszoleges pont)
    SecpPoint scalarMul(const SecpPoint& p, const BigInt& k) const;
    // P / k = (k^{-1} mod n) * P   — Overshoot/szita felezeshez
    SecpPoint scalarDiv(const SecpPoint& p, const BigInt& k) const;

    // Pont == pont (affin)
    static bool pointsEqual(const SecpPoint& a, const SecpPoint& b);

    // Range-Count: Current=start; amig Current!=target: Current+=G, step++
    // left/target = cel, right/start = bazis  (UI: Cel / Start)
    RangeCountResult rangeCountFromTo(const SecpPoint& start, const SecpPoint& target,
                                      uint64_t maxSteps = kRangeCountDefaultLimit) const;

    // Skalar redukcio / inverz a gorbe rendje (n) szerint
    BigInt modN(const BigInt& a) const;
    BigInt invN(const BigInt& a) const;

    static bool parseHexCoord(const std::string& hex, BigInt& out, std::string& err);

    // Compressed (02/03+X) VAGY uncompressed (04+X+Y) pubkey -> affin pont
    bool parsePubkeyAny(const std::string& pubkeyHex, SecpPoint& out, std::string& err) const;

    // Compressed pubkey (02/03 || 32-byte X) -> affin (x,y), y^2 = x^3+7 (mod p)
    bool decompressCompressed(const std::string& compressedHex, SecpPoint& out, std::string& err) const;

    const BigInt& fieldPrime() const { return p_; }
    const BigInt& curveOrder() const { return n_; }

private:
    BigInt p_; // mezo modulus
    BigInt n_; // gorbe rend (skalar modulus)
    BigInt seven_;
    SecpPoint g_;

    BigInt modP(const BigInt& a) const;
    BigInt addP(const BigInt& a, const BigInt& b) const;
    BigInt subP(const BigInt& a, const BigInt& b) const;
    BigInt mulP(const BigInt& a, const BigInt& b) const;
    BigInt invP(const BigInt& a) const;
    BigInt powP(BigInt base, BigInt exp) const;
};
