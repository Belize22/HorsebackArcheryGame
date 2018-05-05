#include <vector>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

using namespace std;

#ifndef Node_H
#define Node_H
#endif

class Node {
	private:
		glm::vec4 color;             //Various properties of each node required to draw the body part's shape and color.
		glm::mat4 modelMatrix;
		glm::mat4 scaleMatrix;
		glm::mat4 rotTransMatrix;
		vector<Node*> children;      //All the body parts that move alongside the current body part in question.

	public:
		Node();
		Node(glm::vec4 &colorParam);
		void addChild(Node* child);
		void setMatrices(glm::mat4 &scaleMatrixParam, glm::mat4 &rotTransMatrixParam);
		void setColor(glm::vec4 &colorParam);
		glm::vec4 getColor();
		glm::mat4 getScaleMatrix();
		glm::mat4 getRotTransMatrix();
		Node* getChildAt(int childParam);
		int getChildQuantity();
		void* operator new(size_t i);
};