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

void PrintDocument(const Document& document);
