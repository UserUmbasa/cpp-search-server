#include "remove_duplicates.h"
void RemoveDuplicates(SearchServer& search_server)
{

	std::map<std::set<std::string>, int> duplicator;
	std::set<std::string> unique;
	std::set<int> Id_to_delete;

	for (const int document_id : search_server) 
	{
		std::map<std::string, double> arr = search_server.GetWordFrequencies(document_id);

		transform(begin(arr), end(arr), inserter(unique, unique.begin()), [](auto String) { return String.first; });

		if(!duplicator.count(unique))
		{
			duplicator.emplace(unique, document_id);
		}
		else
		{
			Id_to_delete.emplace(document_id);
		}
		unique.clear();
	}
	for (auto& id : Id_to_delete)
	{
		std::cout << "Found duplicate document id " << id << std::endl;
		search_server.RemoveDocument(id);
	}
}
