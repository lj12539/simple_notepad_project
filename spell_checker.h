#ifndef SPELL_CHECKER_H
#define SPELL_CHECKER_H

#include <string>
#include <set>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>

class spell_checker {
public:
    explicit spell_checker(const std::string& dictionary_path) {
        std::ifstream file(dictionary_path);
        std::string word;
        while (file >> word) {
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            dict.insert(word);
        }
    }

    std::string clean_word(const std::string& input) const {
        std::string result;
        for (char ch : input) {
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                result += std::tolower(static_cast<unsigned char>(ch));
            }
        }
        return result;
    }

    bool is_correct(const std::string& word) const {
        std::string cleaned = clean_word(word);
        if (cleaned.empty()) return true;
        return dict.find(cleaned) != dict.end();
    }

    std::vector<std::string> get_suggestions(const std::string& word) const {
        std::string cleaned = clean_word(word);
        std::vector<std::string> suggestions;
        if (cleaned.empty()) return suggestions;

        for (const auto& dict_word : dict) {
            if (dict_word.rfind(cleaned.substr(0, 2), 0) == 0 &&
                std::abs(static_cast<int>(dict_word.length() - cleaned.length())) <= 2) {
                suggestions.push_back(dict_word);
                if (suggestions.size() >= 5) break;
                }
        }
        return suggestions;
    }

private:
    std::set<std::string> dict;
};

#endif // SPELL_CHECKER_H