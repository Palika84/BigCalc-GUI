#include "crypto/BitcoinCrypto.hpp"
#include <iostream>
#include <string>
int main() {
  struct { const char* k; const char* pub; } v[] = {
    {"1","0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"},
    {"2","02c6047f9441ed7d6d3045406e95c07cd85c778e4b8cef3ca7abac09b95c709ee5"},
    {"3","02f9308a019258c31049344f85f89d5229b531c845836f99b08601f113bce036f9"},
    {"7","025cbdf0646e5db4eaa398f365f2ea7a0e3d419b7e0330e39ce92bddedcac4f9bc"},
    {"8","022f01e5e15cca351daff3843fb70f3c2f0a1bdd05e5af888a67784ef3e10a2a01"},
  };
  int fails=0;
  for (auto& t: v) {
    auto r=deriveBitcoinKeys(t.k);
    if (!r.ok || r.pubCompressed!=t.pub) { std::cout<<"FAIL pub k="<<t.k<<" got="<<r.pubCompressed<<"\n"; ++fails; }
    else std::cout<<"OK pub k="<<t.k<<" addr="<<r.address<<" wif="<<r.wif<<"\n";
  }
  auto a=deriveBitcoinKeys("1");
  if (a.address!="1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH") { std::cout<<"FAIL addr1\n"; ++fails; }
  if (a.wif!="KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn") { std::cout<<"FAIL wif1\n"; ++fails; }
  auto b=deriveBitcoinKeys("2");
  if (a.wif==b.wif) { std::cout<<"FAIL wif collide\n"; ++fails; }
  std::cout<<(fails?"FAIL\n":"ALL OK\n");
  return fails?1:0;
}
