#include "Node.h"

//Default color is white. Matrices always have a default since both matrices have setters.
Node::Node()
{
	color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
	scaleMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
	rotTransMatrix = glm::translate(modelMatrix, glm::vec3(0.0f))*glm::rotate(modelMatrix, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
}

//Allows one to choose the color of the horse's body part.
Node::Node(glm::vec4 &colorParam)
{
	color = colorParam;
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
	scaleMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
	rotTransMatrix = glm::translate(modelMatrix, glm::vec3(0.0f))*glm::rotate(modelMatrix, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
}

//Establish that a certain body part is a child of the current body part.
void Node::addChild(Node* child)
{
	children.push_back(child);
}

//Setter for both matrices.
void Node::setMatrices(glm::mat4 &scaleMatrixParam, glm::mat4 &rotTransMatrixParam)
{
	scaleMatrix = scaleMatrixParam;
	rotTransMatrix = rotTransMatrixParam;
}

//Setter for color
void Node::setColor(glm::vec4 &colorParam) {
	color = colorParam;
}

//Getter for color
glm::vec4 Node::getColor()
{
	return color;
}

//Getter for scale matrix
glm::mat4 Node::getScaleMatrix()
{
	return scaleMatrix;
}

//Getter for rotation-translation matrix.
glm::mat4 Node::getRotTransMatrix()
{
	return rotTransMatrix;
}

//Retrieve a child of a particular body part.
Node* Node::getChildAt(int i) {
	return children.at(i);
}

//How many body parts attached to parent body part.
int Node::getChildQuantity() {
	return children.size();
}

//Allows new instances of node to have the proper byte boundaries (they change since glm::mat4 parameters are passed)!
void* Node::operator new(size_t i)
{
	return _mm_malloc(i, 16);
}