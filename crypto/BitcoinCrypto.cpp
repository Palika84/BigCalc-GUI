#include "BitcoinCrypto.hpp"
#include "SecpAffine.hpp"
#include "BigInt.hpp"
#include "../Lang.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

// ---- SHA-256 ----
void sha256_transform(uint32_t* state, const uint8_t* data) {
    uint32_t W[64];
    for (int i = 0; i < 16; ++i) {
        W[i] = (uint32_t(data[i * 4]) << 24) | (uint32_t(data[i * 4 + 1]) << 16) |
               (uint32_t(data[i * 4 + 2]) << 8) | uint32_t(data[i * 4 + 3]);
    }
    static const uint32_t K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = ((W[i - 15] >> 7) | (W[i - 15] << 25)) ^ ((W[i - 15] >> 18) | (W[i - 15] << 14)) ^ (W[i - 15] >> 3);
        uint32_t s1 = ((W[i - 2] >> 17) | (W[i - 2] << 15)) ^ ((W[i - 2] >> 19) | (W[i - 2] << 13)) ^ (W[i - 2] >> 10);
        W[i] = W[i - 16] + s0 + W[i - 7] + s1;
    }
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = ((e >> 6) | (e << 26)) ^ ((e >> 11) | (e << 21)) ^ ((e >> 25) | (e << 7));
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t temp1 = h + S1 + ch + K[i] + W[i];
        uint32_t S0 = ((a >> 2) | (a << 30)) ^ ((a >> 13) | (a << 19)) ^ ((a >> 22) | (a << 10));
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;
        h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256(const uint8_t* data, size_t len, uint8_t out[32]) {
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    uint8_t buf[64] = {0};
    size_t original_len = len;
    while (len >= 64) {
        sha256_transform(state, data);
        data += 64;
        len -= 64;
    }
    size_t i = 0;
    for (; i < len; ++i) buf[i] = data[i];
    buf[i++] = 0x80;
    if (i > 56) {
        while (i < 64) buf[i++] = 0;
        sha256_transform(state, buf);
        std::memset(buf, 0, 64);
        i = 0;
    }
    while (i < 56) buf[i++] = 0;
    uint64_t bits = uint64_t(original_len) * 8;
    for (int j = 0; j < 8; ++j)
        buf[56 + j] = uint8_t(bits >> ((7 - j) * 8));
    sha256_transform(state, buf);
    for (int j = 0; j < 8; ++j) {
        out[j * 4] = uint8_t((state[j] >> 24) & 0xFF);
        out[j * 4 + 1] = uint8_t((state[j] >> 16) & 0xFF);
        out[j * 4 + 2] = uint8_t((state[j] >> 8) & 0xFF);
        out[j * 4 + 3] = uint8_t(state[j] & 0xFF);
    }
}

void sha256d(const uint8_t* data, size_t len, uint8_t out[32]) {
    uint8_t mid[32];
    sha256(data, len, mid);
    sha256(mid, 32, out);
}

// ---- RIPEMD-160 ----
uint32_t rol(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }

void ripemd160(const uint8_t* msg, size_t len, uint8_t out[20]) {
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476, h4 = 0xC3D2E1F0;

    auto f = [](int j, uint32_t x, uint32_t y, uint32_t z) -> uint32_t {
        if (j < 16) return x ^ y ^ z;
        if (j < 32) return (x & y) | (~x & z);
        if (j < 48) return (x | ~y) ^ z;
        if (j < 64) return (x & z) | (y & ~z);
        return x ^ (y | ~z);
    };
    static const int r[80] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        7,4,13,1,10,6,15,3,12,0,9,5,2,14,11,8,
        3,10,14,4,9,15,8,1,2,7,0,6,13,11,5,12,
        1,9,11,10,0,8,12,4,13,3,7,15,14,5,6,2,
        4,0,5,9,7,12,2,10,14,1,3,8,11,6,15,13
    };
    static const int rr[80] = {
        5,14,7,0,9,2,11,4,13,6,15,8,1,10,3,12,
        6,11,3,7,0,13,5,10,14,15,8,12,4,9,1,2,
        15,5,1,3,7,14,6,9,11,8,12,2,10,0,4,13,
        8,6,4,1,3,11,15,0,5,12,2,13,9,7,10,14,
        12,15,10,4,1,5,8,7,6,2,13,14,0,3,9,11
    };
    static const int s[80] = {
        11,14,15,12,5,8,7,9,11,13,14,15,6,7,9,8,
        7,6,8,13,11,9,7,15,7,12,15,9,11,7,13,12,
        11,13,6,7,14,9,13,15,14,8,13,6,5,12,7,5,
        11,12,14,15,14,15,9,8,9,14,5,6,8,6,5,12,
        9,15,5,11,6,8,13,12,5,12,13,14,11,8,5,6
    };
    static const int ss[80] = {
        8,9,9,11,13,15,15,5,7,7,8,11,14,14,12,6,
        9,13,15,7,12,8,9,11,7,7,12,7,6,15,13,11,
        9,7,15,11,8,6,6,14,12,13,5,14,13,13,7,5,
        15,5,8,11,14,14,6,14,6,9,12,9,12,5,15,8,
        8,5,12,9,12,5,14,6,8,13,6,5,15,13,11,11
    };
    static const uint32_t K[5]  = {0x00000000,0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xA953FD4E};
    static const uint32_t KK[5] = {0x50A28BE6,0x5C4DD124,0x6D703EF3,0x7A6D76E9,0x00000000};

    std::vector<uint8_t> buf(msg, msg + len);
    uint64_t bitlen = uint64_t(len) * 8;
    buf.push_back(0x80);
    while ((buf.size() % 64) != 56) buf.push_back(0);
    for (int i = 0; i < 8; ++i) buf.push_back(uint8_t((bitlen >> (8 * i)) & 0xFF));

    for (size_t off = 0; off < buf.size(); off += 64) {
        uint32_t X[16];
        for (int i = 0; i < 16; ++i) {
            X[i] = uint32_t(buf[off + i * 4]) | (uint32_t(buf[off + i * 4 + 1]) << 8) |
                   (uint32_t(buf[off + i * 4 + 2]) << 16) | (uint32_t(buf[off + i * 4 + 3]) << 24);
        }
        uint32_t al = h0, bl = h1, cl = h2, dl = h3, el = h4;
        uint32_t ar = h0, br = h1, cr = h2, dr = h3, er = h4;
        for (int j = 0; j < 80; ++j) {
            uint32_t t = rol(al + f(j, bl, cl, dl) + X[r[j]] + K[j / 16], s[j]) + el;
            al = el; el = dl; dl = rol(cl, 10); cl = bl; bl = t;
            t = rol(ar + f(79 - j, br, cr, dr) + X[rr[j]] + KK[j / 16], ss[j]) + er;
            ar = er; er = dr; dr = rol(cr, 10); cr = br; br = t;
        }
        uint32_t t = h1 + cl + dr;
        h1 = h2 + dl + er;
        h2 = h3 + el + ar;
        h3 = h4 + al + br;
        h4 = h0 + bl + cr;
        h0 = t;
    }
    auto store = [&](uint32_t v, int idx) {
        out[idx] = uint8_t(v & 0xFF);
        out[idx + 1] = uint8_t((v >> 8) & 0xFF);
        out[idx + 2] = uint8_t((v >> 16) & 0xFF);
        out[idx + 3] = uint8_t((v >> 24) & 0xFF);
    };
    store(h0, 0); store(h1, 4); store(h2, 8); store(h3, 12); store(h4, 16);
}

// ---- Base58Check ----
std::string base58Encode(const uint8_t* data, size_t len) {
    static const char* alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    int zeros = 0;
    while (zeros < (int)len && data[zeros] == 0) ++zeros;

    std::vector<uint8_t> b(data, data + len);
    std::string enc;
    while (!b.empty() && !(b.size() == 1 && b[0] == 0)) {
        int rem = 0;
        std::vector<uint8_t> next;
        next.reserve(b.size());
        for (uint8_t byte : b) {
            int acc = rem * 256 + byte;
            int q = acc / 58;
            rem = acc % 58;
            if (!next.empty() || q != 0) next.push_back(uint8_t(q));
        }
        enc.push_back(alphabet[rem]);
        b.swap(next);
    }
    for (int i = 0; i < zeros; ++i) enc.push_back('1');
    std::reverse(enc.begin(), enc.end());
    return enc;
}

std::string base58Check(const std::vector<uint8_t>& payload) {
    uint8_t hash[32];
    sha256d(payload.data(), payload.size(), hash);
    std::vector<uint8_t> full = payload;
    full.insert(full.end(), hash, hash + 4);
    return base58Encode(full.data(), full.size());
}

std::string toHex(const uint8_t* data, size_t len) {
    static const char* hexd = "0123456789abcdef";
    std::string s;
    s.resize(len * 2);
    for (size_t i = 0; i < len; ++i) {
        s[i * 2] = hexd[data[i] >> 4];
        s[i * 2 + 1] = hexd[data[i] & 0xF];
    }
    return s;
}

bool parsePrivToBytes(const std::string& in, uint8_t out[32], std::string& err) {
    std::string s = in;
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ')) s.pop_back();
    size_t a = 0;
    while (a < s.size() && s[a] == ' ') ++a;
    s = s.substr(a);
    if (s.empty()) { err = T(Msg::BtcEmptyPrivKey); return false; }

    bool neg = false;
    if (s[0] == '-') { neg = true; s = s.substr(1); }
    if (neg) { err = T(Msg::BtcNegativePrivKey); return false; }

    // Hex: 0x... vagy tiszta hex ha csak a-f jegyek (de decimalis elosegitett)
    bool asHex = false;
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        asHex = true;
        s = s.substr(2);
    }

    std::memset(out, 0, 32);
    if (asHex) {
        if (s.empty()) { err = T(Msg::BtcEmptyHexKey); return false; }
        // csak hex jegyek
        for (char c : s) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                err = T(Msg::BtcInvalidHexDigitInKey);
                return false;
            }
        }
        if (s.size() > 64) { err = T(Msg::BtcKeyLargerThan256); return false; }
        // balrol pad 0-val 64-re
        std::string padded(64 - s.size(), '0');
        padded += s;
        for (int i = 0; i < 32; ++i) {
            out[i] = uint8_t(std::stoul(padded.substr(i * 2, 2), nullptr, 16));
        }
    } else {
        // decimalis -> big-endian 32 byte
        for (char c : s) {
            if (c < '0' || c > '9') { err = T(Msg::BtcInvalidDecimalKey); return false; }
            uint32_t carry = uint32_t(c - '0');
            for (int i = 31; i >= 0; --i) {
                uint64_t cur = uint64_t(out[i]) * 10ull + carry;
                out[i] = uint8_t(cur & 0xFF);
                carry = uint32_t(cur >> 8);
            }
            if (carry) { err = T(Msg::BtcKeyLargerThan256); return false; }
        }
    }

    bool allZero = true;
    for (int i = 0; i < 32; ++i) if (out[i]) { allZero = false; break; }
    if (allZero) { err = T(Msg::BtcPrivKeyZero); return false; }

    // curve order N
    static const uint8_t N[32] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
        0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x41
    };
    for (int i = 0; i < 32; ++i) {
        if (out[i] < N[i]) break;
        if (out[i] > N[i]) { err = T(Msg::BtcPrivKeyGERange); return false; }
        if (i == 31) { err = T(Msg::BtcPrivKeyGERange); return false; }
    }
    return true;
}

} // namespace

BitcoinKeyInfo deriveBitcoinKeys(const std::string& privDecOrHex) {
    BitcoinKeyInfo info;
    uint8_t priv[32];
    if (!parsePrivToBytes(privDecOrHex, priv, info.error)) return info;

    try {
        // BigInt skalár a helyes SecpAffine k*G motorhoz (a regi Jacobian hibas volt k>1-re)
        BigInt key = BigInt::zero();
        for (int i = 0; i < 32; ++i)
            key = key * BigInt(256) + BigInt(priv[i]);

        SecpAffine secp;
        SecpPoint pubPt = secp.scalarMulBase(key);
        if (pubPt.infinity) {
            info.error = T(Msg::BtcPubkeyInfinityInvalidScalar);
            return info;
        }

        std::string pubHex = pubPt.compressedHex();
        uint8_t pub[33];
        for (int i = 0; i < 33; ++i)
            pub[i] = static_cast<uint8_t>(std::stoul(pubHex.substr(static_cast<size_t>(i * 2), 2), nullptr, 16));

        info.privHex = toHex(priv, 32);
        info.pubCompressed = pubHex;

        // WIF compressed mainnet: 0x80 || priv || 0x01
        std::vector<uint8_t> wifPayload;
        wifPayload.push_back(0x80);
        wifPayload.insert(wifPayload.end(), priv, priv + 32);
        wifPayload.push_back(0x01);
        info.wif = base58Check(wifPayload);

        // P2PKH address: Base58Check(0x00 || RIPEMD160(SHA256(pub)))
        uint8_t sha[32];
        sha256(pub, 33, sha);
        uint8_t h160[20];
        ripemd160(sha, 32, h160);
        std::vector<uint8_t> addrPayload;
        addrPayload.push_back(0x00);
        addrPayload.insert(addrPayload.end(), h160, h160 + 20);
        info.address = base58Check(addrPayload);

        info.ok = true;
    } catch (const std::exception& ex) {
        info.ok = false;
        info.error = ex.what();
    }
    return info;
}

BitcoinKeyInfo deriveFromCompressedPub(const std::string& compressedPubHex) {
    BitcoinKeyInfo info;
    try {
        std::string hex;
        for (char c : compressedPubHex) {
            if (std::isxdigit(static_cast<unsigned char>(c)))
                hex.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        if (hex.size() != 66 || hex[0] != '0' || (hex[1] != '2' && hex[1] != '3')) {
            info.error = T(Msg::BtcCompressedPubNeeds66Hex);
            return info;
        }
        uint8_t pub[33];
        for (int i = 0; i < 33; ++i)
            pub[i] = static_cast<uint8_t>(std::stoul(hex.substr(static_cast<size_t>(i * 2), 2), nullptr, 16));

        info.pubCompressed = hex;
        info.wif = "-"; // nincs privat kulcs
        info.privHex = "";

        uint8_t sha[32];
        sha256(pub, 33, sha);
        uint8_t h160[20];
        ripemd160(sha, 32, h160);
        std::vector<uint8_t> addrPayload;
        addrPayload.push_back(0x00);
        addrPayload.insert(addrPayload.end(), h160, h160 + 20);
        info.address = base58Check(addrPayload);
        info.ok = true;
    } catch (const std::exception& ex) {
        info.error = ex.what();
    }
    return info;
}
