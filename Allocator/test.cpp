#include <vector>
#include <iostream>
#include "VectorAllocator.h"
using namespace std;

int main ( ) {
	vector<int, VectorAllocator<int>> test;
	for (int i = 0; i < 100; i++) {
		test.push_back (i);
	}
	for (int i = 0; i < 100; i++) {
		cout << test[i] << endl;
	}
	system ("pause");
	return 0;
}
