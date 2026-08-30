#pragma once

#include <array>
#include <cstdint>
#include <string>

struct BitcoinKeyInfo {
    bool ok = false;
    std::string error;
    std::string privHex;          // 64 hex chars
    std::string wif;              // compressed mainnet WIF
    std::string pubCompressed;    // 66 hex chars (33 bytes)
    std::string address;          // P2PKH Base58Check
};

// privDecOrHex: tiszta decimalis egesz VAGY 0x... hex (a kalkulator egesz eredmenye)
BitcoinKeyInfo deriveBitcoinKeys(const std::string& privDecOrHex);

// Csak pubkeybol (G-mod eredmeny): WIF nincs, address + compressed pub van
BitcoinKeyInfo deriveFromCompressedPub(const std::string& compressedPubHex);
