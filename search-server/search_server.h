#pragma once
#include <tuple>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <execution>
#include <string>
#include <vector>
#include <type_traits>
#include <random>
#include <future>  
#include "concurrent_map.h"
#include "read_input_functions.h"
#include "string_processing.h"
#include "log_duration.h" 

const double accuracy = 1e-6;
const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
    template <typename StringContainer>
    SearchServer(const StringContainer& stop_words);
    SearchServer(const std::string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)) {}
    SearchServer(std::string_view& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)) {}


    void AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::string_view raw_query,DocumentPredicate document_predicate) const;
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::execution::sequenced_policy&, std::string_view raw_query, DocumentPredicate document_predicate) const;
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::execution::parallel_policy&, std::string_view raw_query, DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(const std::execution::sequenced_policy&, std::string_view raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(const std::execution::parallel_policy&, std::string_view raw_query, DocumentStatus status) const;


    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;
    std::vector<Document> FindTopDocuments(const std::execution::sequenced_policy&,
        std::string_view raw_query) const;
    std::vector<Document> FindTopDocuments(const std::execution::parallel_policy&,
        std::string_view raw_query) const;


    void RemoveDocument(int document_id);
    void RemoveDocument(const std::execution::sequenced_policy&,
        int document_id);
    void RemoveDocument(const std::execution::parallel_policy&,
        int document_id);

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query,
        int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&,
        std::string_view raw_query,
        int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&,
        std::string_view raw_query,
        int document_id) const;

    int GetDocumentCount() const;

    std::set<int>::const_iterator begin() const;
    std::set<int>::const_iterator end() const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

private:
    struct DocumentData {
        std::string data_string_;
        int rating;
        DocumentStatus status;
    };
    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };
    struct Query {
        std::vector<std::string_view> plus_words_;
        std::vector<std::string_view> minus_words_;
    };
    const std::set<std::string, std::less<>> stop_words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> document_to_word_freqs;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;

    bool IsStopWord(std::string_view word) const;
    static bool IsValidWord(std::string_view word);

    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);
    QueryWord ParseQueryWord(std::string_view& text) const;
    Query ParseQuery(std::string_view& text) const;
    double ComputeWordInverseDocumentFreq(std::string_view& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query,
        DocumentPredicate document_predicate) const;
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const std::execution::sequenced_policy&,
        const Query& query,
        DocumentPredicate document_predicate) const;
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const std::execution::parallel_policy&,
        const Query& query,
        DocumentPredicate document_predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid");
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    return FindTopDocuments(std::execution::seq,
        raw_query,
        document_predicate);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::execution::sequenced_policy&,
    std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    const auto temp = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(std::execution::seq,
        temp,
        document_predicate);

    sort(std::execution::seq,
        matched_documents.begin(),
        matched_documents.end(),
        [this](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < accuracy) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::execution::parallel_policy&,
    std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    const auto temp = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(std::execution::par,
        temp,
        document_predicate);

    sort(std::execution::par,
        matched_documents.begin(),
        matched_documents.end(),
        [this](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < accuracy) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
    DocumentPredicate document_predicate) const {
    return FindAllDocuments(std::execution::seq,
        query,
        document_predicate);
}
template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const std::execution::sequenced_policy&,
    const Query& query,
    DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;

    for (std::string_view word : query.plus_words_) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id,
                document_data.status,
                document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const auto word : query.minus_words_) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back({ document_id,
                                     relevance,
                                     documents_.at(document_id).rating });
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const std::execution::parallel_policy&,
    const Query& query,
    DocumentPredicate document_predicate) const {
    const int COUNT = 101;
    ConcurrentMap<int, double> document_to_relevance(COUNT);

    const auto plus_func = [this,
        &document_predicate,
        &document_to_relevance] (std::string_view word) {
        if (word_to_document_freqs_.count(word) == 0) {//
            return;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);//
            if (document_predicate(document_id,
                document_data.status,
                document_data.rating)) {
                document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
            }
        }
    };

    for_each(std::execution::par,
        query.  plus_words_.begin(),
        query.plus_words_.end(),
        plus_func);

    const auto minus_erase_func = [&](std::string_view word) {
        if (word_to_document_freqs_.count(word) == 0) {
            return;
        }
        for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    };

    for_each(std::execution::par,
        query.minus_words_.begin(),
        query.minus_words_.end(),
        minus_erase_func);

    const auto& document_to_relevance_bom = document_to_relevance.BuildOrdinaryMap();

    std::vector<Document> matched_documents;
    for (const auto& [document_id, relevance] : document_to_relevance_bom) {
        matched_documents.push_back({ document_id,
                                     relevance,
                                     documents_.at(document_id).rating });
    }

    return matched_documents;
}
