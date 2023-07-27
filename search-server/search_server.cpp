#include <numeric>
#include "search_server.h"

void SearchServer::AddDocument(int document_id,
    std::string_view document,
    DocumentStatus status,
    const std::vector<int>& ratings) {

    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid ID");
    }

    auto [id, data] = documents_.emplace(document_id,
        DocumentData{
            std::string(document),
            ComputeAverageRating(ratings),
            status
        });
    document_ids_.insert(document_id);

    const auto words = SplitIntoWordsNoStop(id->second.data_string_);

    const double inv_word_count = 1.0 / words.size();

    for (auto word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        document_to_word_freqs[document_id][word] += inv_word_count;
    }
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
    DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq, raw_query, status);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&,
    std::string_view raw_query,
    DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq,
        raw_query,
        [status](int document_id,
            DocumentStatus document_status,
            int rating) {
                return document_status == status;
        }
    );
}

std::vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&,
    std::string_view raw_query,
    DocumentStatus status) const {
    return FindTopDocuments(std::execution::par,
        raw_query,
        [status](int document_id,
            DocumentStatus document_status,
            int rating) {
                return document_status == status;
        }
    );
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq,
        raw_query,
        DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&,
    std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq,
        raw_query,
        DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&,
    std::string_view raw_query) const {
    return FindTopDocuments(std::execution::par,
        raw_query,
        DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::set<int>::const_iterator SearchServer::begin() const {
    return document_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const {
    return document_ids_.end();
}

const std::map<std::string_view, double >& SearchServer::GetWordFrequencies(int document_id) const
{
    if (document_to_word_freqs.count(document_id) != 0)
    {
        return  document_to_word_freqs.at(document_id);
    }
    static std::map<std::string_view, double > empty_word_frequencies{};
    return empty_word_frequencies;
}


void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(std::execution::seq,
        document_id);
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&,
    int document_id) {
    auto temp = std::find(
        document_ids_.begin(),
        document_ids_.end(),
        document_id);

    if (temp == document_ids_.end()) {
        return;
    }
    else {
        document_ids_.erase(temp);
    }

    documents_.erase(document_id);

    std::for_each(word_to_document_freqs_.begin(),
        word_to_document_freqs_.end(),
        [&](auto& temp) {
            temp.second.erase(document_id);
        });
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&,
    int document_id) {
    auto temp = std::find(std::execution::par,
        document_ids_.begin(),
        document_ids_.end(),
        document_id);

    if (temp == document_ids_.end()) {
        return;
    }
    else {
        document_ids_.erase(temp);
    }

    documents_.erase(document_id);

    std::for_each(std::execution::par,
        word_to_document_freqs_.begin(),
        word_to_document_freqs_.end(),
        [&](auto& temp) {
            temp.second.erase(document_id);
        });
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query,
    int document_id) const {
    return MatchDocument(std::execution::seq,
        raw_query,
        document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&,
    std::string_view raw_query,
    int document_id) const {

    if ((document_id < 0) || (documents_.count(document_id) <= 0)) {
        throw std::invalid_argument("Non-existent ID");
    }

    const auto result = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;

    for (auto word : result.minus_words_) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return { {}, documents_.at(document_id).status };
        }
    }

    for (auto word : result.plus_words_) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }

    return { matched_words,
            documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&,
    std::string_view raw_query,
    int document_id) const {

    if ((document_id < 0) || (documents_.count(document_id) <= 0)) {
        throw std::invalid_argument("Non-existent ID");
    }

    const auto& result = ParseQuery(raw_query);

    const auto& check = [this, document_id](std::string_view word) {
        const auto temp = word_to_document_freqs_.find(word);
        return temp != word_to_document_freqs_.end() &&
            temp->second.count(document_id);
    };

    if (std::any_of(std::execution::par,
        result.minus_words_.begin(),
        result.minus_words_.end(),
        check)) {
        return { {}, documents_.at(document_id).status };
    }

    std::vector<std::string_view> matched_words(result.plus_words_.size());

    auto end = std::copy_if(std::execution::par,
        result.plus_words_.begin(),
        result.plus_words_.end(),
        matched_words.begin(),
        check);

    std::sort(matched_words.begin(), end);
    end = std::unique(std::execution::par,
        matched_words.begin(),
        end);

    matched_words.erase(end, matched_words.end());
    return { matched_words,
            documents_.at(document_id).status };
}

bool SearchServer::IsStopWord(std::string_view word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(std::string_view word) {
    return std::none_of(word.begin(),
        word.end(),
        [](char c) {
            return c >= '\0' && c < ' ';
        }
    );
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
    std::vector<std::string_view> words;

    for (auto& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word is invalid");
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(),
        ratings.end(),
        0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view& text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty");
    }

    bool is_minus = false;

    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }

    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw std::invalid_argument("Query word is invalid");
    }
    return { text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(std::string_view& text) const {
    Query result;

    for (std::string_view& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words_.push_back(query_word.data);
            }
            else {
                result.plus_words_.push_back(query_word.data);
            }
        }
    }

    std::sort(std::execution::par,
        result.minus_words_.begin(),
        result.minus_words_.end());
    std::sort(std::execution::par,
        result.plus_words_.begin(),
        result.plus_words_.end());

    result.minus_words_.erase(std::unique(result.minus_words_.begin(),
        result.minus_words_.end()),
        result.minus_words_.end());
    result.plus_words_.erase(std::unique(result.plus_words_.begin(),
        result.plus_words_.end()),
        result.plus_words_.end());

    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(std::string_view& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
