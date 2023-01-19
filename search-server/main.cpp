#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;                    // Теперь релевантность — вещественное число, вычисляемое по формуле TF - IDF.
};

struct Query {
    set<string> minuswords;
    set<string> pluswords;
};

class SearchServer {
public:



    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(const int& document_id, const string& document)
    {
        ++document_count_;
        const vector<string> words = SplitIntoWordsNoStop(document);

        for (string word : words)
        {
            double tf = 0;
             tf += 1.0 / words.size();
           
          
            word_to_document_freqs_[word][document_id] += 1.0 / words.size();
           // word_to_document_freqs_[word].insert({ document_id, tf});           //3.1
            /*for (auto [id, tf] : word_to_document_freqs_[word]) 
            {
                cout << word << " "s << id <<" "s << tf << endl;
            }    */       
           // cout << word << " "s << document_id << " "s << tf << endl;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {

        Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    int document_count_ = 0;                                           // кол-во документов 

    map<string, map<int, double>> word_to_document_freqs_;           //слово документа, id документа где находится слово и ТФ  
    

    set<string> stop_words_;
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            // cout << "0"s << word << endl;
            if (word[0] != '-') {
                query_words.pluswords.insert(word);
                // cout << "+"s <<  word << endl;
            }
            else {
                query_words.minuswords.insert(word.substr(1));
                //  cout << "-"s <<  word.substr(1) << endl;
            }
        }

        return query_words;
    }

    vector<Document> FindAllDocuments(Query &query_words) const            // 4.1
    {
        vector<Document> matched_documents;
        map<int, double> document_to_relevance;                       
        vector<int> doc_delete;
        
     
        for (const auto& documentp : query_words.pluswords)
        {
                //cout << documentp << endl;
             if (word_to_document_freqs_.count(documentp) != 0)
             {
                 for (const auto& [id, tf] : word_to_document_freqs_.at(documentp))
                 {
                     document_to_relevance[id] += tf * log(document_count_ * 1.0 / word_to_document_freqs_.at(documentp).size());
                 }
             }
        }

        for (const string& word : query_words.minuswords)
        {
            if (word_to_document_freqs_.count(word) == 0) 
            {
                continue;
            }
            for (const auto [wordq, id_tf] : word_to_document_freqs_.at(word)) 
            {
                document_to_relevance.erase(wordq);
            }
        }
        
        for (auto& [id, relevance] : document_to_relevance)
        {
            matched_documents.push_back({ id, relevance });
            //cout << id << " "s << relevance << endl;
        }

        return matched_documents;
    }

    /* static int MatchDocument(const DocumentContent& content, const Query query_words) {

         if (query_words.pluswords.empty()) 
         {
             return 0;
         }

         set<string> matched_words;

         for (const string& word : content.words) 
         {

             if (matched_words.count(word) != 0) 
             {
                 continue;
             }
             if (query_words.minuswords.count(word) != 0) 
             {
                 return 0;
             }

             if (query_words.pluswords.count(word) != 0) 
             {
                 matched_words.insert(word);
             }
         }
         return static_cast<int>(matched_words.size());
     }*/
};


SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
   
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
    
    const string query = ReadLine();//считывает запрос -+слова
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", " << "relevance = "s << relevance << " }"s << endl;
    }
}

