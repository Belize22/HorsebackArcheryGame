#include "Stack.h"

//Push a matrix on top of the stack.
void Stack::push(glm::mat4 &matrixToPush)
{
	matrixList.push_back(matrixToPush);
}

//Grab the matrix off the top of the stack.
glm::mat4 Stack::pop()
{
	glm::mat4 matrixToPop = matrixList.back();
	matrixList.pop_back();
	return matrixToPop;
}

//Get the matrix on top of the stack but don't take it off.
glm::mat4 Stack::top()
{
	return matrixList.back();
}