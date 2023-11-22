#include <vector>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <random>

std::vector<std::string> generate_words(const std::string& fileName) {
    std::fstream wordFile;
    std::vector<std::string> words;
    wordFile.open(fileName, std::ios::in);
    if (wordFile.is_open()) {
        std::string word;

        while (getline(wordFile, word)) {
            words.push_back(word);
        }
    }

    wordFile.close();
    return words;
}

std::string generate_passage(std::vector<std::string> &words, int length){
    // default_random_engine generator;
    std::random_device rd;
    std::mt19937 e{rd()};
    std::uniform_int_distribution<int> distribution(0, words.size() - 1);

    int index;
    std::string passage;
    for (int i = 0; i < length; i++) {
        index = distribution(rd);
        for (const auto &character : words[index]) {
            passage.push_back(character);
        }
        passage.push_back(' ');
    }
    return passage;
}
