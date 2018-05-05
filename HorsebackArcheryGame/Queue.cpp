#include "Queue.h"

//Let item in queue.
void Queue::enqueue(int id) {
	queueList.push_back(id);
}

//Let item in front of queue leave.
int Queue::dequeue() {
	if (getSize() > 0) {
		int front = queueList.at(0);
		queueList.erase(queueList.begin());
		return front;
	}
	return -1;
}

//Remove element from queue (not conventional but important so that we can keep track of proper collisions that leave).
void Queue::removeElement(int id) {
	for (int i = 0; i < getSize(); i++)
		if (queueList.at(i) == id) {
			queueList.erase(queueList.begin() + i);
			break;
		}
}

//Get item at front of queue.
int Queue::getFront() {
	return queueList.at(0);
}

//Get size of queue.
int Queue::getSize() {
	return queueList.size();
}

//Get element of queue (not conventional but needed for collision lookups).
int Queue::getElement(int pos) {
	return queueList.at(pos);
}