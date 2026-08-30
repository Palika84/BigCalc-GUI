#include "crypto/BitcoinCrypto.hpp"
#include <iostream>
#include <string>

int main() {
    auto r = deriveBitcoinKeys("1");
    if (!r.ok) {
        std::cout << "FAIL: " << r.error << "\n";
        return 1;
    }

    const std::string expectPub =
        "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798";
    const std::string expectWif =
        "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn";
    const std::string expectAddr = "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH";

    int fails = 0;
    if (r.pubCompressed != expectPub) {
        std::cout << "PUB mismatch\n  got " << r.pubCompressed << "\n  exp " << expectPub << "\n";
        ++fails;
    }
    if (r.wif != expectWif) {
        std::cout << "WIF mismatch\n  got " << r.wif << "\n  exp " << expectWif << "\n";
        ++fails;
    }
    if (r.address != expectAddr) {
        std::cout << "ADDR mismatch\n  got " << r.address << "\n  exp " << expectAddr << "\n";
        ++fails;
    }

    if (fails) return 1;
    std::cout << "BTC tests OK\n";
    return 0;
}
