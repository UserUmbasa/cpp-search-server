#include "test_user.h"


using namespace std;


//генерация случайных оценок
vector <int> Evaluations()
{
	vector<int>arr;
	for(int i = 0; i < 3; ++i)
	{
		arr.push_back(rand() % 10);
	}
	return arr;
}


void Manual_Input_String()
{
	cout << "Введите стоп слова: ";
	SearchServer search_server(ReadLine());
	cout << "Введите кол -во документов: ";
	const int document_count = ReadLineWithNumber();
	for (int document_id = 0; document_id < document_count; ++document_id)
	{
		cout << "Введите документ номер " << document_id << " ";
		search_server.AddDocument(document_id,ReadLine(), DocumentStatus::ACTUAL, Evaluations());
	}
	cout << "Введите запрос: ";
	const auto search_results = search_server.FindTopDocuments(ReadLine());
	int page_size = 2;
	auto pages = Paginate(search_results, page_size);
	cout << endl;

	// Выводим найденные документы по страницам
	for (auto page = pages.begin(); page != pages.end(); ++page) {
		cout << *page << endl;
		cout << "Page break"s << endl;
	}
}
void Machine_input_String()
{
	SearchServer search_server("and with"s);
	search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

	const auto search_results = search_server.FindTopDocuments("curly dog"s);
	int page_size = 2;
	auto pages = Paginate(search_results, page_size);

	// Выводим найденные документы по страницам
	for (auto page = pages.begin(); page != pages.end(); ++page) {
		cout << *page << endl;
		cout << "Page break"s << endl;
	}
}
void Machine_input_Container()
{
	vector<string> arr{ "and", "with" };
	SearchServer search_server(arr);
	search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

	const auto search_results = search_server.FindTopDocuments("curly dog"s);
	int page_size = 2;
	auto pages = Paginate(search_results, page_size);

	// Выводим найденные документы по страницам
	for (auto page = pages.begin(); page != pages.end(); ++page) {
		cout << *page << endl;
		cout << "Page break"s << endl;
	}

}
void Machine_input_EmptyRequest()
{
	SearchServer search_server("and in at"s);
	RequestQueue request_queue(search_server);

	search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

	// 1439 запросов с нулевым результатом
	for (int i = 0; i < 1439; ++i) {
		request_queue.AddFindRequest("empty request"s);
	}
	// все еще 1439 запросов с нулевым результатом
	request_queue.AddFindRequest("curly dog"s);
	// новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
	request_queue.AddFindRequest("big collar"s);
	// первый запрос удален, 1437 запросов с нулевым результатом
	request_queue.AddFindRequest("sparrow"s);
	cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
}
