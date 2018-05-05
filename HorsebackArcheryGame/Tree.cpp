#include "Tree.h"

//Incase the user wants to set the root later than when initializing the tree.
Tree::Tree()
{
	root = NULL;
}

//Set the root from the get go.
Tree::Tree(Node* rootParam, GLuint objectColorLocationParam, GLuint transformLocationParam, GLuint VAOparam, int drawTypeParam)
{
	root = rootParam;
	objectColorLocation = objectColorLocationParam;
	transformLocation = transformLocationParam;
	VAO = VAOparam;
	drawType = drawTypeParam;
}

//Draw the horse by doing a pre-order traversal starting from the root.
void Tree::drawTraversalFromRoot(Stack* scaleMatrixStack, Stack* rotTransMatrixStack)
{
	drawTraversal(root, scaleMatrixStack, rotTransMatrixStack);
}

//Draw the specified body part and it's children by doing a pre-order traversal.
void Tree::drawTraversal(Node* node, Stack* scaleMatrixStack, Stack* rotTransMatrixStack)
{
	//set color
	//if (!texturesActive)
	glUniform4f(objectColorLocation, node->getColor().x, node->getColor().y, node->getColor().z, node->getColor().w);
	scaleMatrixStack->push(node->getScaleMatrix());                                                                         //Keep track of the current matrices being used.
	rotTransMatrixStack->push(node->getRotTransMatrix());
	glBindVertexArray(VAO);
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(rotTransMatrixStack->top()*scaleMatrixStack->top())); //Utilize scale, then rotate, then translate by getting matrices on top of each stack
	glDrawArrays(drawType, 0, 36);
	glBindVertexArray(0);
	for (int i = 0; i < node->getChildQuantity(); i++)                                                                      //Start drawing the child body parts.
		drawTraversal(node->getChildAt(i), scaleMatrixStack, rotTransMatrixStack);
	scaleMatrixStack->pop();                                                                                               //Get rid of current matrices being used so parent can utilize proper matrices.
	rotTransMatrixStack->pop();
}

//Set where the horse is drawn.
void Tree::setTransformLocation(int transformLocationParam) {
	transformLocation = transformLocationParam;
}

//Set how the horse is rendered.
void Tree::setDrawType(int drawTypeParam) {
	drawType = drawTypeParam;
}