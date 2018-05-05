#include <vector>

using namespace std;

#ifndef Queue_H
#define Queue_H
#endif

class Queue {
	private:
		vector<int> queueList;
	public:
		void enqueue(int id);
		int dequeue();
		void removeElement(int id);
		int getFront();
		int getSize();
		int getElement(int pos);
};