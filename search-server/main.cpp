#include "search_server.h"
#include"paginator.h"
#include"request_queue.h"
#include "test_user.h"
#include "remove_duplicates.h"
using namespace std;


int main()
{
    setlocale(LC_ALL, "Russian");

    // Machine_input_EmptyRequest();
	SearchServer search_server("and with"s);
	AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
	// дубликат документа 2, будет удалён
	AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
	// отличие только в стоп-словах, считаем дубликатом
	AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
	// множество слов такое же, считаем дубликатом документа 1
	AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
	// добавились новые слова, дубликатом не является
	AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
	// множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
	AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });
	// есть не все слова, не является дубликатом
	AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });
	// слова из разных документов, не является дубликатом
	AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

	// 1) Откажитесь от метода GetDocumentId(int index) и вместо него определите методы begin и end.
	//for (const int document_id : search_server) {
		// ...
	//}
    
	// 2) Разработайте метод получения частот слов по id документа :
	//auto result = search_server.GetWordFrequencies(2);
    
	// 3) Разработайте метод удаления документов из поискового сервера
	//search_server.RemoveDocument(3);

	cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
	RemoveDuplicates(search_server);
	cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;	
}
