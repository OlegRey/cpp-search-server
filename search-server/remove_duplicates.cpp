#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {

    std::set<std::set<std::string>> unique_words; 
    std::vector<int> duplicate_id;

    for (const int document_id : search_server) {
        auto word_freqs = search_server.GetWordFrequencies(document_id);
        std::set<std::string> words;
        for (const auto& [word, freq] : word_freqs) {

            words.insert(word); 
        }
             
        if (!unique_words.emplace(words).second) { 
            duplicate_id.push_back(document_id);
        }
    }

    for (const int document_id : duplicate_id) {
        std::cout << "Found duplicate document id "s << document_id << '\n';
        search_server.RemoveDocument(document_id);
    }
}


  