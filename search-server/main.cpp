#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
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
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
        const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }


    template<typename Predicate>
    vector<Document> FindTopDocuments(const string& raw_query, Predicate predicate) const
    {
        const Query query = ParseQuery(raw_query);
        vector<Document> matched_documents = FindAllDocuments(query, predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        return FindTopDocuments(raw_query,
            [](int document_id, DocumentStatus status, int rating)
            {
                return status == DocumentStatus::ACTUAL;
            });
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus document_status) const
    {
        return FindTopDocuments(raw_query,
            [document_status](int document_id, DocumentStatus status, int rating)
            {
                return status == document_status;
            });
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
        int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
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

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

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

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template<typename Predicate>
    vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& ref = documents_.at(document_id);
                if (predicate(document_id, ref.status, ref.rating))
                {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }


};

/////////////////////////////////////Tests start here/////////////////////////////////




void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
    const string& hint) {
    if (!value) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
    const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

template <typename TestFunc>
void RunTestImpl(TestFunc t_func, const string& func) {
    t_func();
    cerr << func << " OK" << endl;
}


#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define RUN_TEST(func) RunTestImpl((func), #func)



void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
            "Stop words must be excluded from documents"s);
    }
}


void TestOfAddingDocuments()
{
    int doc_id = 42;
    string content = "a blue cat with beautiful green eyes"s;
    vector<int> ratings = { 1, 2, 3 };
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

    auto found_docs = server.FindTopDocuments("cat"s);
    ASSERT(found_docs.size() == 1);

    doc_id = 43;
    content = "a white cat with blue eyes"s;
    ratings = { 2, 3, 4 };
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

    found_docs = server.FindTopDocuments("cat"s);
    ASSERT(found_docs.size() == 2);

    doc_id = 44;
    content = "a white dog with a long tail"s;
    ratings = { 2, 3, 4 };
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

    found_docs = server.FindTopDocuments("dog"s);
    ASSERT_HINT(found_docs.size() == 1,
        "Server must not find documents that do not contain the query word");

    found_docs = server.FindTopDocuments("dog and cat"s);
    ASSERT_HINT(found_docs.size() == 3,
        "Server must find documents by all the query words");

    found_docs = server.FindTopDocuments("children playing basketball"s);
    ASSERT_HINT(found_docs.size() == 0,
        "Server must not add documents if it is not demanded");
}

void TestMinusWords()
{
    int doc_id = 42;
    string  content = "a blue cat with green eyes"s;
    vector<int> ratings = { 1, 2, 3 };
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

    {
        auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT(found_docs.size() == 1);
    }

    {
        auto found_docs = server.FindTopDocuments("cat -green"s);
        ASSERT_HINT(found_docs.empty(),
            "Server must pay attention to minus words");
    }

    {
        content = "a cool cat";
        doc_id = 1;
        ratings = { 2, 6, 7 };
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto found_docs = server.FindTopDocuments("cat -green"s);
        ASSERT_HINT(found_docs.size() == 1,
            "Server must pay attention to minus words and do not find documents that contain such words");
    }
}

void TestOfCorrectRelevanceSorting()
{

    SearchServer server;
    server.AddDocument(1, "cat with dog", DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "cat and cat", DocumentStatus::ACTUAL, { 2, 3, 4 });
    server.AddDocument(3, "cat and cat and one more cat", DocumentStatus::ACTUAL, { 3, 4, 5 });

    auto found_docs = server.FindTopDocuments("cat and"s);

    ASSERT_HINT(found_docs[0].id == 2, "Incorrect realisation of sorting documents by id");
    ASSERT_HINT(found_docs[1].id == 3, "Incorrect realisation of sorting documents by id");
    ASSERT_HINT(found_docs[2].id == 1, "Incorrect realisation of sorting documents by id");

}

void TestAverageRating()
{
    SearchServer server;
    server.AddDocument(1, "cat with dog", DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "cat and cat", DocumentStatus::ACTUAL, { 2, 3, 4 });
    server.AddDocument(3, "cat and cat and one more cat", DocumentStatus::ACTUAL, { 3, 4, 5 });

    auto found_docs = server.FindTopDocuments("cat and"s);

    ASSERT_HINT(found_docs[0].rating == 3, "Incorrect realisation of computing tha average rating");
    ASSERT_HINT(found_docs[1].rating == 4, "Incorrect realisation of computing tha average rating");
    ASSERT_HINT(found_docs[2].rating == 2, "Incorrect realisation of computing tha average rating");
}

void TestWorkWithPredicate()
{
    SearchServer server;
    server.AddDocument(1, "cat with dog", DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "cat and cat", DocumentStatus::ACTUAL, { 2, 3, 4 });
    server.AddDocument(3, "cat and cat and one more cat", DocumentStatus::ACTUAL, { 3, 4, 5 });

    auto found_docs = server.FindTopDocuments("cat and"s, [](int document_id, DocumentStatus status, int rating)
        {
            return document_id % 2 == 1;
        });
    ASSERT_HINT(found_docs.size() == 2,
        "Incorrect realisation of working with a custom predicate");

    found_docs = server.FindTopDocuments("cat and"s, [](int document_id, DocumentStatus status, int rating)
        {
            return document_id % 2 == 0;
        });
    ASSERT_HINT(found_docs.size() == 1,
        "Incorrect realisation of working with a custom predicate");
}

void TestSearchWithStatus()
{
    SearchServer server;
    server.AddDocument(1, "cat with dog", DocumentStatus::BANNED, { 1, 2, 3 });
    server.AddDocument(2, "cat and cat", DocumentStatus::ACTUAL, { 2, 3, 4 });
    server.AddDocument(3, "cat and cat and one more cat", DocumentStatus::ACTUAL, { 3, 4, 5 });

    auto found_docs = server.FindTopDocuments("cat and"s, [](int document_id, DocumentStatus status, int rating)
        {
            return status == DocumentStatus::BANNED;
        });
    ASSERT_HINT(found_docs.size() == 1,
        "Incorrect realisation of finding documents with a custom status");

    found_docs = server.FindTopDocuments("cat and"s);
    ASSERT_HINT(found_docs.size() == 2,
        "Incorrect realisation of finding documents with a custom status");
}

void TestOfCorrectRelevanceComputing()
{
    SearchServer server;
    server.AddDocument(1, "white cat and trendy collar", DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "fluffy cat fluffy tail", DocumentStatus::ACTUAL, { 2, 3, 4 });
    server.AddDocument(3, "groomed dog beautiful eyes", DocumentStatus::ACTUAL, { 3, 4, 5 });

    double rel1 = 0.0, rel2 = 0.0, rel3 = 0.0;

    rel1 = server.FindTopDocuments("fluffy groomed cat")[0].relevance;
    rel2 = server.FindTopDocuments("fluffy groomed cat")[1].relevance;
    rel3 = server.FindTopDocuments("fluffy groomed cat")[2].relevance;

    ASSERT_EQUAL_HINT((floor(rel1 * 10000) / 10000), 0.6506, "The realisation of the relevance computing is wrong");
    ASSERT_EQUAL_HINT((floor(rel2 * 10000) / 10000), 0.2746, "The realisation of the relevance computing is wrong");
    ASSERT_EQUAL_HINT((floor(rel3 * 10000) / 10000), 0.0810, "The realisation of the relevance computing is wrong");
}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestOfAddingDocuments);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestOfCorrectRelevanceSorting);
    RUN_TEST(TestAverageRating);
    RUN_TEST(TestWorkWithPredicate);
    RUN_TEST(TestSearchWithStatus);
    RUN_TEST(TestOfCorrectRelevanceComputing);
}

///////////////////////////////////Tests end here///////////////////////////////////
void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating
        << " }"s << endl;
}
int main() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }
    cout << "ACTUAL:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; })) {
        PrintDocument(document);
    }
    cout << "Even ids:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }
    return 0;
}