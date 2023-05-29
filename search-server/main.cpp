#include <iostream>
#include "search_server.h"
#include "paginator.h"
#include "log_duration.h"
#include "request_queue.h"
#include "string_processing.h"
#include "remove_duplicates.h"
#include "read_input_functions.h"
#include "request_queue.h"
#include "test_example_functions.h"



using namespace std;


int main() {
    SearchServer search_server("and with"s);
    AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, 3, "funny pet and nasty rat pet"s, DocumentStatus::ACTUAL, { 7, 2, 7 });


    RemoveDuplicates(search_server);
    auto a = search_server.GetWordFrequencies(3);

}
