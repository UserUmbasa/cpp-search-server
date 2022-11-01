// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:
#include <iostream>

using namespace std;

int main() {
	
	int num = 0;

	for (int i = 3; i <= 1000; i += 10) { ++num; }
	// cout<<i<<" "s<<num<<endl;
    cout<<num<<endl; 
}
// Закомитьте изменения и отправьте их в свой репозиторий.
