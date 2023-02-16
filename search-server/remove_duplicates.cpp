#include "remove_duplicates.h"
void RemoveDuplicates(SearchServer& search_server)
{

	std::map<std::set<std::string>, int> duplicator;
	std::set<std::string> temp;
	std::vector<int> num;

	for (const int document_id : search_server) 
	{
		std::map<std::string, double> arr = search_server.GetWordFrequencies(document_id);

		for (auto it = begin(arr);it!=end(arr);++it )
		{
			temp.emplace(it->first);
		}
		if(!duplicator.count(temp))
		{
			duplicator.emplace(temp, document_id);
		}
		else if (duplicator.lower_bound(temp)->second > document_id)
		{
			duplicator.emplace(temp, document_id);
		}
		else
		{
			num.push_back(document_id);
		}
		temp.clear();
	}
	for (auto& i : num)
	{
		std::cout << "Found duplicate document id " << i << std::endl;
		search_server.RemoveDocument(i);
	}
}
