#include "sort.h"

#include <iostream>
#include <string>
#include <vector>

int main()
{

    std::vector<int> numbers {
        8, 2, 6, 5, 1, 9, 3
    };

    std::cout << "Sorting integers with my::sort "
              << "(default operator<):\n";

    std::cout << "Before: ";

    for (int n : numbers) {
        std::cout << n;

        if (n != numbers.back()) {
            std::cout << " ";
        }
    }
    std::cout << "\n";

    my::sort(numbers.begin(),
        numbers.end());

    std::cout << "After: ";

    for (int n : numbers) {
        std::cout << " " << n;
    }

    std::cout << "\n\n";

    std::vector<std::string> languages {
        "Java",
        "C++",
        "Kotlin",
        "C#",
        "Scala",
        "C"
    };

    std::cout << "Sorting strings with my::sort "
              << "(default operator<):\n";

    std::cout << "Before: ";

    for (const auto& s : languages) {

        std::cout << s;

        if (s != languages.back()) {
            std::cout << " ";
        }
    }

    std::cout << "\n";

    my::sort(languages.begin(),
        languages.end());

    std::cout << "After: ";

    for (const auto& s : languages) {
        std::cout << " " << s;
    }

    std::cout << "\n\n";

    std::vector<std::pair<std::string, int>> freq {
        { "apple", 2 },
        { "banana", 5 },
        { "cherry", 1 },
        { "date", 3 }
    };

    std::cout << "Sorting word frequencies with "
              << "my::sort "
              << "(lambda: by count descending):\n";

    std::cout << "Before: ";

    for (const auto& p : freq) {

        std::cout << "{ \"" << p.first
                  << "\": "
                  << p.second
                  << " }";

        if (p != freq.back()) {
            std::cout << " ";
        }
    }

    std::cout << "\n";

    my::sort(
        freq.begin(),
        freq.end(),
        [](const auto& a,
            const auto& b) {
            return a.second > b.second;
        });

    std::cout << "After: ";

    for (const auto& p : freq) {

        std::cout << " { \"" << p.first
                  << "\": "
                  << p.second
                  << " }";
    }

    std::cout << "\n";

    return 0;
}