#include "search_server.h"



SearchServer::SearchServer(const std::string str)
{
    SearchServer::stop_words_ = MakeUniqueNonEmptyStrings(SplitIntoWords(str));
    if (!all_of(SearchServer::stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument({ "Some of stop words are invalid" });
    }
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
    const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument({ "Invalid document_id" });
    }
    const auto words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        document_to_word_freqs[document_id][word] += inv_word_count; ///////////////////////////////////////

    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.push_back(document_id);
}
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}
std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument({ "Word is invalid" });
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}
bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}
SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const
{
    if (text.empty()) {
        throw std::invalid_argument({ "Query word is empty" });
    }
    std::string word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument({ "Query word is invalid" });
    }

    return { word, is_minus, IsStopWord(word) };
}
SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
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
std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (SearchServer::word_to_document_freqs_.count(word) == 0) {
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
bool SearchServer::IsValidWord(const std::string& word)
{
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c)
        {
            return c >= '\0' && c < ' ';
        });
}
int SearchServer::ComputeAverageRating(const std::vector<int>& ratings)
{
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}
double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
// Разработайте метод получения частот слов по id документа: 
std::map<std::string, double> SearchServer::GetWordFrequencies(int document_id) const
{
    if(document_to_word_freqs.count(document_id)!=0)
    {
        return document_to_word_freqs.at(document_id);
    }
    static std::map<std::string, double> empty_word_frequencies{};
    return empty_word_frequencies;    
}
std::vector<int>::const_iterator SearchServer::begin() const
{
    return document_ids_.begin();
}

std::vector<int>::const_iterator SearchServer::end() const
{
    return document_ids_.end();
}

std::vector<int>::iterator SearchServer::begin()
{
    return document_ids_.begin();
}

std::vector<int>::iterator SearchServer::end()
{
    return document_ids_.end();
}
// 3) Разработайте метод удаления документов из поискового сервера
void SearchServer::RemoveDocument(int document_id)
{
    SearchServer::documents_.erase(document_id);
    SearchServer::document_to_word_freqs.erase(document_id);
    for (auto i :SearchServer::word_to_document_freqs_)
    {
        auto temp = i.second;
        if(temp.count(document_id))
        {
            temp.erase(document_id);
        }
    }
    auto it = std::lower_bound(std::begin(SearchServer::document_ids_), std::end(SearchServer::document_ids_), document_id);
    SearchServer::document_ids_.erase(it);
}
