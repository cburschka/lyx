#include "../lstrings.h"

#include <iostream>

using namespace lyx::support;

using namespace std;

void test_lowercase()
{
	cout << lowercase('A') << endl;
	cout << lowercase("AlLe") << endl;
}

void test_uppercase()
{
	cout << uppercase('a') << endl;
	cout << uppercase("AlLe") << endl;
}

int main() 
{
	test_lowercase();
	test_uppercase();
}
