#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    std::map<std::vector<std::string>, int> unique_words;
    std::vector<int> duplicate_id;

    for (const int document_id : search_server) {
        auto word_freqs = search_server.GetWordFrequencies(document_id);
        std::vector<std::string> words;
       
        for (const auto& [word, freq] : word_freqs) {
            words.push_back(word);
        }

        auto [words_it, emplaced] = unique_words.emplace(words, document_id);
        if (!emplaced) {
            duplicate_id.push_back(document_id);
        }
    }

    for (const int document_id : duplicate_id) {
        std::cout << "Found duplicate document id "s << document_id << '\n';
        search_server.RemoveDocument(document_id);
    }
}