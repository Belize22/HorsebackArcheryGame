#include <vector>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

using namespace std;

#ifndef Stack_H
#define Stack_H
#endif

class Stack {
	private:
		vector<glm::mat4> matrixList;
	public:
		void push(glm::mat4 &matrixToPush);
		glm::mat4 pop();
		glm::mat4 top();
};