#include "..\glew\glew.h"	//include GL Extension Wrangler
#include "..\glfw\glfw3.h"	//include GLFW helper library

#include "Stack.h"
#include "Node.h"

class Tree {
	private:
		Node* root;
		GLuint objectColorLocation;
		GLuint transformLocation;
		GLuint VAO;
		int drawType;
	public:
		Tree();
		Tree(Node* rootParam, GLuint objectColorLocationParam, GLuint transformLocationParam, GLuint VAOParam, int drawTypeParam);
		void drawTraversalFromRoot(Stack* scaleMatrixStack, Stack* rotTransMatrixStack);
		void drawTraversal(Node* node, Stack* scaleMatrixStack, Stack* rotTransMatrixStack);
		void setTransformLocation(int transformLocationParam);
		void setDrawType(int drawTypeParam);
};