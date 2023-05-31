#include <set>
#include <vector>

#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server)
{
    std::set<std::set<std::string>> unique_documents;
    std::vector<int> ids_to_delete;
    for (const int id : search_server) {
        const map<std::string, double> word_freqs = search_server.GetWordFrequencies(id);
        set<std::string> words;
        transform(word_freqs.begin(), word_freqs.end(), inserter(words, words.begin()),
            [](const pair<std::string, double> word)
            {
                return word.first;
            });

        if (unique_documents.count(words) == 0)
        {
            unique_documents.insert(words);
        }
        else
        {
            ids_to_delete.push_back(id);
        }
    }

    for (const int id : ids_to_delete)
    {
        cout << "Found duplicate document id " << id << endl;
        search_server.RemoveDocument(id);
    }
}
