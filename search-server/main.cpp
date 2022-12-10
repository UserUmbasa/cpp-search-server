#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#include "search_server.h"

using namespace std;
// -------- Начало модульных тестов поисковой системы ----------

void TestExcludeStopWordsFromAddedDocumentContent() 
{
    const int doc_id = 42;
    const string content = "cat in the city das"s;
    const vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        ASSERT(doc0.id == doc_id);
    }
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

void TestMinusWordsInQuery()
{
    const int doc_id = 42;
    const string content = "cat in the city das"s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id_ = 43;
    const string content_ = "cat in the pussy city ret das"s;
    const vector<int> ratings_ = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_, content_, DocumentStatus::ACTUAL, ratings_);
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT(found_docs.size() == 2);
        const auto found_docs_ = server.FindTopDocuments("cat -ret"s);
        ASSERT(found_docs_.size() == 1);
    }
}

void TestStatus()
{
    const int doc_id = 42;
    const string content = "cat in the city das"s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id_ = 43;
    const string content_ = "cat in the pussy city ret das"s;
    const vector<int> ratings_ = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_, content_, DocumentStatus::BANNED, ratings_);
        const auto found_docs = server.FindTopDocuments("cat ret"s, DocumentStatus::ACTUAL);
        ASSERT(found_docs.size() == 1);

    }
}
void TestPredicate()
{
    const int doc_id = 3;
    const string content = "cat in the city das"s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id_ = 43;
    const string content_ = "cat in the pussy city ret das"s;
    const vector<int> ratings_ = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_, content_, DocumentStatus::BANNED, ratings_);
        const auto found_docs = server.FindTopDocuments("cat ret"s,
            [](int document_id, DocumentStatus status, int rating) { return document_id == 3; });
        ASSERT(found_docs.size() == 1);
    }

}
void TestRelevance()
{
    const int doc_id_1 = 1;
    const string content_1 = "cat in the city "s;
    const vector<int> ratings_1 = { 1, 2, 3 };

    const int doc_id_2 = 2;
    const string content_2 = "cat in pussy city das"s;
    const vector<int> ratings_2 = { 1, 2, 3 };

    const int doc_id_3 = 3;
    const string content_3 = "cat in the pussy city ret das"s;
    const vector<int> ratings_3 = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        const auto found_docs = server.FindTopDocuments("cat ret das"s);
        ASSERT(found_docs[0].relevance > found_docs[1].relevance && found_docs[1].relevance > found_docs[2].relevance);
    }
}
void TestRating()
{
    const int doc_id = 1;
    const string content = "cat in the pussy city ret das"s;
    const vector<int> ratings = { 2, 2, -9 };
    int rat = 0;
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        if (ratings.empty())
        {
            rat = 0;
        }
        for (const int rating : ratings)
        {
            rat += rating;
        }
        rat = rat / static_cast<int>(ratings.size());
        const auto found_docs = server.FindTopDocuments("cat ret das"s);
        ASSERT(found_docs[0].rating == rat);
    }
}
// -------- конец модульных тестов поисковой системы ----------
void TestSearchServer() 
{
    TestExcludeStopWordsFromAddedDocumentContent();
    TestMinusWordsInQuery();
    TestStatus();
    TestPredicate();
    TestRelevance();
    TestRating();
}
int main() 
{
    TestSearchServer();
    cout << "Search server testing finished"s << endl;
}
