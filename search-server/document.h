#pragma once
#include<string>
#include<set>
#include<iostream>



enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};
struct Document {
    Document() = default;
    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};
struct DocumentData {
    int rating;
    DocumentStatus status;
};
struct QueryWord {
    std::string data;
    bool is_minus;
    bool is_stop;
};
struct Query {
    std::set<std::string> plus_words;
    std::set<std::string> minus_words;
};
void PrintDocument(const Document& document);
