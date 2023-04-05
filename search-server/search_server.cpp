#include "search_server.h"

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) { //v1
   
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }

    const std::vector<std::string> words = SplitIntoWordsNoStop(document);
    for (auto& word : words) {
        word_to_document_freqs_[word][document_id] += 1.0 / words.size();
    }
    for (auto& word : words) {
        document_to_word_freqs_[document_id][word] += 1.0 / words.size();
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.emplace(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [&status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

//int SearchServer::GetDocumentId(int index) const { //удалить
//    return document_ids_.at(index);
//}

auto SearchServer::begin() const noexcept -> std::set<int>::const_iterator {
    return document_ids_.cbegin();
}

auto SearchServer::end() const noexcept -> std::set<int>::const_iterator {
    return document_ids_.cend();
}
std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const { //v1
    static const std::map<std::string, double> empty_request;
    return document_to_word_freqs_.count(document_id) == 0 ? empty_request : document_to_word_freqs_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {      //v1
    document_to_word_freqs_.erase(document_id);
    document_ids_.erase(document_id);
    documents_.erase(document_id);
    
    //documents_.erase(document_ids_.erase(document_to_word_freqs_.erase(document_id)));
}
bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {return c >= '\0' && c < ' '; });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word "s + word + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

 SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
    
     if (text.empty()) {
         throw std::invalid_argument("Query word is empty"s);
     }
     std::string word = text;
     bool is_minus = false;
     if (word[0] == '-') {
         is_minus = true;
         word = word.substr(1);
     }
     if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
         throw std::invalid_argument("Query word "s + text + " is invalid");
     }
     return { word, is_minus, IsStopWord(word) };
 }
  
    SearchServer::Query SearchServer::ParseQuery(const std::string & text) const {
        Query result;
        for (const std::string& word : SplitIntoWords(text)) {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    result.minus_words.insert(query_word.data);
                }
                else {
                    result.plus_words.insert(query_word.data);
                }
            }
        }
        return result;
    }

   // Existence required 
    double SearchServer::ComputeWordInverseDocumentFreq(const std::string & word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
