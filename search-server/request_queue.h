#pragma once
#include "search_server.h"


class RequestQueue
{
public:
    explicit RequestQueue(const SearchServer& search_server) : server(search_server), no_result_requests_counter_(0)
    {

    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:

    struct QueryResult
    {
        uint64_t number_of_answers;
        uint64_t current_time;
    };

    std::deque<QueryResult> requests_;
    const SearchServer& server;
    int no_result_requests_counter_;
    const static int min_in_day_ = 1440;

    void AddRequest(const size_t answers_amount);

    void ClearOldRequests();

};

template<typename DocumentPredicate>
inline std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
{
    vector<Document> results = server.FindTopDocuments(raw_query, document_predicate);
    AddRequest(results.size());
    return results;
}