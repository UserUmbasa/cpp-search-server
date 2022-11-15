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

string ReadLine() 
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() 
{
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) 
{
    vector<string> words;
    string word;
    for (const char c : text) 
    {
        if (c == ' ') 
        {
            if (!word.empty()) 
            {                                                
                words.push_back(word);
                word.clear();
            }
        }
        else 
            {
                word += c;
            }
    }
    if (!word.empty()) 
    {
        words.push_back(word);
    }

    return words;
}

struct Document 
{
    int id;
    double relevance;
};

class SearchServer 
{
public:
    void SetStopWords(const string& text) 
    {
        for (const string& word : SplitIntoWords(text)) 
        {
            stop_words_.insert(word);
        }
    }


    void AddDocument(int document_id, const string& document)
    {
        const vector<string> words = SplitIntoWordsNoStop(document);
        double count_iD = 1.0 / words.size();//доля слова в документе

        for (const string& word : words) 
        {       
            if (word_to_document_freqs_.count(word)) 
            { 
                word_to_document_freqs_[word][document_id] += count_iD; 
            }
            else { 
                    word_to_document_freqs_.insert({ {word},{ {document_id,count_iD} } }); 
                 }
            // кол-во документов (т.к первый по адресу ноль (сделаем его 1)
            id_count = document_id + 1;
        }


    }

    vector<Document> FindTopDocuments(const string& raw_query) const 
    {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query);

        sort(matched_documents.begin(), matched_documents.end(),[](const Document& lhs, const Document& rhs) 
        {
            return lhs.relevance > rhs.relevance;
        });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) 
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:

    struct Query 
    {
        set<string> minus_word;
        set<string> plus_word;
    };

    map<string, map<int, double>> word_to_document_freqs_;
    int id_count;
    set<string> stop_words_;


    bool IsStopWord(const string& word) const 
    {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const
    {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) 
        {
            if (!IsStopWord(word)) 
            {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const 
    {
        Query minus_plus_;

        for (const string& word : SplitIntoWordsNoStop(text))
        {
            if (word[0] == '-') 
            { 
                minus_plus_.minus_word.insert(word.substr(1)); 
            } else {
                        minus_plus_.plus_word.insert(word);
                   }
        }
        return minus_plus_;
    }



    vector<Document> FindAllDocuments(const Query& query) const
    {
        vector<Document> matched_documents;
        map<int, double> document_to_relevance;

        if (!query.plus_word.empty())
        { 
            for (const string& word : query.plus_word)
            {
                if (word_to_document_freqs_.count(word))
                {
                    double IDF_TF = 0.0;
                    double IDF = log (static_cast<double>(id_count) / word_to_document_freqs_.at(word).size());
                    for (const auto& [id, tf] : word_to_document_freqs_.at(word))
                    {   
                        IDF_TF = tf * IDF; 
                        if (document_to_relevance.count(id)) 
                        { 
                            document_to_relevance.at(id) += IDF_TF; IDF_TF = 0.0; 
                        }
                        else 
                            { 
                                document_to_relevance.insert({ id,IDF_TF }); IDF_TF = 0.0;
                            }
                    }
                }
            }
        }
        if (!query.minus_word.empty())
        {
            for (const string& word : query.minus_word)
            {
                if (word_to_document_freqs_.count(word))
                {
                    for (const auto& [id, tf] : word_to_document_freqs_.at(word)) 
                    { 
                        document_to_relevance.erase(id); 
                    }
                }
            }
        }
        for (const auto& [id, rel] : document_to_relevance) 
        { 
            matched_documents.push_back({ id,rel }); 
        }
        return matched_documents;

    }


};

SearchServer CreateSearchServer() 
{
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) 
    {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() 
{
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) 
    {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }
}