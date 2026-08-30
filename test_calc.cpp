#include "Calculator.hpp"
#include <iostream>

static int fails = 0;

static void expect(const std::string& expr, int prec, const std::string& dec, const std::string& hex) {
    auto r = evaluate(expr, prec);
    if (!r.ok) {
        std::cout << "FAIL " << expr << " error=" << r.error << "\n";
        ++fails;
        return;
    }
    if (r.decimal != dec || r.hex != hex) {
        std::cout << "FAIL " << expr << "\n  got dec=" << r.decimal << " hex=" << r.hex
                  << "\n  exp dec=" << dec << " hex=" << hex << "\n";
        ++fails;
    } else {
        std::cout << "OK   " << expr << " => " << r.decimal << " | " << r.hex << "\n";
    }
}

int main() {
    expect("10 + 5", 3, "15.000", "0xF.000");
    expect("10 - 5", 3, "5.000", "0x5.000");
    expect("10 * 5", 3, "50.000", "0x32.000");
    expect("10 / 4", 3, "2.500", "0x2.800");
    expect("0xFF * 1.5", 3, "382.500", "0x17E.800");
    expect("0x10 + 0x1", 0, "17", "0x11");
    if (fails) {
        std::cout << fails << " failed\n";
        return 1;
    }
    std::cout << "All tests passed.\n";
    return 0;
}
