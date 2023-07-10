#include "process_queries.h"
#include <execution>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> to_return(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), to_return.begin(), [&search_server](std::string raw)
        {
            return search_server.FindTopDocuments(raw);
        });
    return to_return;
}

std::list<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries)
{
    std::list<Document> documents;
    for (auto& same_query_docs : ProcessQueries(search_server, queries))
    {
        for (auto& doc : same_query_docs)
        {
            documents.push_back(doc);
        }
    }
    return documents;
}