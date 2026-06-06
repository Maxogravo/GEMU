//Denary to hexadecimal converter tool
#include <iostream>
#include <string>

int main() {
    unsigned long long n;
    std::cout << "Enter Denary Number: ";
    std::cin >> n;

    if (n == 0) {
        std::cout << "0 \n";
        return 0;
    }

    std::string hex = "";
    char digits[] = "0123456789ABCDEF";

    while (n > 0) {
        int rem = n % 16;
        hex = digits[rem] + hex;
        n = n / 16;
    }

    std::cout << hex << "\n";

    return 0;
}
