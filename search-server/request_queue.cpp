#include "request_queue.h"

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status)
{
    std::vector<Document> results = server.FindTopDocuments(raw_query, status);
    AddRequest(results.size());
    return results;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query)
{
    std::vector<Document> results = server.FindTopDocuments(raw_query);
    AddRequest(results.size());
    return results;
}

int RequestQueue::GetNoResultRequests() const
{
    return no_result_requests_counter_;
}

void RequestQueue::AddRequest(const size_t answers_amount)
{
    ClearOldRequests();
    if (answers_amount == 0) { ++no_result_requests_counter_; }
    requests_.push_back({ answers_amount, 1 });
}

void RequestQueue::ClearOldRequests()
{
    if (!requests_.empty())
    {
        for (QueryResult& request : requests_)
        {
            ++request.current_time;
        }
        if (requests_.front().current_time > min_in_day_)
        {
            requests_.pop_front();
            --no_result_requests_counter_;
        }
    }
}