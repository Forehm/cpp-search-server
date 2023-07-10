#pragma once

#include "document.h"
#include "search_server.h"

#include <random>
#include <string>
#include <vector>

void AddDocument(SearchServer& search_server, int document_id, const std::string& document,
    DocumentStatus status = DocumentStatus::ACTUAL, const std::vector<int>& ratings = {});