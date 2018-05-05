#include "Tree.h"
#include "Queue.h"

enum status { normal, stopped, avoiding, controlled };
enum forecastDirection { leftDir, straightDir, rightDir, noDir };
enum animation { run, walk, jump };

class Horse {
	private:
		glm::vec4 WHITE = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);      //Color of a normal horse (and normal collision status when debugging).
		glm::vec4 BLUE = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);       //Color of a horse that is stopping during a collision (debugging only).
		glm::vec4 YELLOW = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);     //Color of a horse that is avoiding during a collision (debugging only).
		glm::vec4 FUCHSIA = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);    //Color of a horse when selected by the user.
		glm::vec4 PURPLE = glm::vec4(0.25f, 0.0f, 0.75f, 1.0f);   //Color of a horse when controlled by the user.

		//Constants
		const float PI = 3.14f;
		const float MIN_SPEED = 0.25f;
		const float MAX_SPEED = 1.0f;
		const float CHANGE_ANIMATION_SPEED = 0.7f;
		const float DEGREES_TO_TURN = 30*PI/180;
		const float RUN_SPEED_MULTIPLIER = 6.0f;
		const float WALK_SPEED_MULTIPLIER = 4.0f;
		const float JUMP_SPEED_MULTIPLIER = 5.0f;
		const int JUMP_FRAMES = 46;

		//Properties involving how the horse is drawn.
		GLuint objectColorLocation;
		GLuint transformLocation;
		GLuint VAO;
		int drawType;
		glm::mat4 modelMatrix;
		glm::mat4 worldRotation;

		bool debugCollisionStatus;

		//Properties entailing horse behaviour and horse control
		bool isSelected;
		bool isControlled;
		bool isStopped;
		
		//Properties of the horse itself.
		int id;
		float posX;
		float posY;
		float posZ;
		float pan;
		float tilt;
		float speed;
		animation animationType;
		glm::vec4 color;

		//Properties entailing how many steps horses take before they turn.
		int steps;
		int currentSteps;
		int stepToChangeSpeedAt;  //Change speed at specific step if applicable.
		int stepToStopAt;         //Stop horse at specific step if applicable.
		bool doWeChangeSpeed;     //Do we change speed for current set of steps?
		bool doWeStop;            //Do we stop for current set of steps?
		int currentStopFrame;     //Current frame while horse is stopped.
		int stopFrames;           //Amount of frames horse stops for.
		int direction;            //1 if left, -1 if right (multiplier for actual turn code entailing rotation).

		//Properties involving collision detection. 
		forecastDirection avoidingDirection; //Where to go during collision.
		bool directionAssigned;              //Given a direction to go during collision yet?
		float radiansTurnedInCollision;      //How often a horse turns during collision.
		Queue* collisionQueue;               //List of horses current horse is collided with. 
		float collisionRadius;
		status overallStatus;                //Can either be normal, stopping due to collision, avoiding due to collision or controlled by the user

		//Properties entailing hierarchical modeling and animations.
		float jointAngles[10];
		float jointSpeed[10];
		float jointDirection[10];

		//Properties entailing scale;
		float scale;
		float scaleOffset;

		//Horse itself.
		Tree* horse;

		//Horse body parts.
		Node* horseTorso;
		Node* horseNeck;
		Node* horseHead;
		Node* horseLeftUpperArm;
		Node* horseRightUpperArm;
		Node* horseLeftUpperLeg;
		Node* horseRightUpperLeg;
		Node* horseLeftLowerArm;
		Node* horseRightLowerArm;
		Node* horseLeftLowerLeg;
		Node* horseRightLowerLeg;

		Stack* scaleMatrixStack;       //Stack for scale related transformations
		Stack* rotTransMatrixStack;    //Stack for rotation and translation related transformations (scale is not part of this. doing this results in shears!).

		//Transformations related to scaling. Separate from rotation and translation to avoid shears.
		glm::mat4 horseTorsoScale;
		glm::mat4 horseNeckScale;
		glm::mat4 horseHeadScale;
		glm::mat4 horseLimbScale;

		//Transformations related to translations and rotations. The main focus of the hierarchy model utilized. Translations account for change in scale too.
		glm::mat4 horseTorsoRot;
		glm::mat4 horseNeckRot;
		glm::mat4 horseHeadRot;
		glm::mat4 horseLeftUpperArmRot;
		glm::mat4 horseLeftLowerArmRot;
		glm::mat4 horseRightUpperArmRot;
		glm::mat4 horseRightLowerArmRot;
		glm::mat4 horseLeftUpperLegRot;
		glm::mat4 horseLeftLowerLegRot;
		glm::mat4 horseRightUpperLegRot;
		glm::mat4 horseRightLowerLegRot;

		//FUNCTIONS ACCESSIBLE FROM CLASS ITSELF.
		int randomNumber(int min, int max);
		glm::mat4 rotateOffset(float x, float y, float z);
		void updateMatrices();
		void setColor(glm::vec4 &colorParam);
		void randomSpeedChange();
		bool progressFrame();
		void checkBounds();
		void executeAnimation();
		animation getAnimationType();
		void setAnimationType(animation animationTypeParam);
		void runAnimation();
		void runAnimationSetup();
		void runArmAnimation(int lowerLimb, int upperLimb);
		void runLegAnimation(int lowerLimb, int upperLimb);
		void runNeckAnimation(int head, int neck);
		void walkAnimation();
		void walkAnimationSetup();
		void walkArmAnimation(int lowerLimb, int upperLimb);
		void walkLegAnimation(int lowerLimb, int upperLimb);
		void walkNeckAnimation(int head, int neck);
		void jumpAnimation();
		void jumpAnimationSetup();
		void jumpArmAnimation(int lowerLimb, int upperLimb);
		void jumpLegAnimation(int lowerLimb, int upperLimb);
		void jumpNeckAnimation(int head, int neck);
	public:
		//CONSTRUCTORS
		Horse();
		Horse(GLuint objectColorLocationParam, GLuint transformLocationParam, GLuint VAOParam, int drawTypeParam, int idParam);

		//FUNCTIONS RELATED TO DRAWING THE HORSE ITSELF.
		void draw();
		void updatePosition();

		//GETTERS
		glm::vec3 getPosition();
		float getCollisionRadius();
		status getCollisionStatus();
		int getId();
		forecastDirection getAvoidingDirection();
		bool getDirectionAssigned();
		bool getIsHorseStopped();
		glm::vec3 getForecastedPosition(forecastDirection direction);

		//SETTERS
		void setCollisionStatus(status statusParam);
		void setWorldRotation(glm::mat4 &worldRotationParam);
		void setDrawType(int drawTypeParam);
		void setAvoidingDirection(forecastDirection direction);
		void setDirectionAssigned(bool directionAssignedParam);
		void setIsSelected(bool isSelectedParam);
		void setIsControlled(bool isControlledParam);
		void setDebugCollisionStatus(bool statusParam);

		//FUNCTIONS RELATED TO OTHER HORSE PROPERTIES.
		void randomizePosition();
		void setStraightPathProperties();
		bool doCollisionsExist();
		bool isTrapped();
		void stopHorse();
		void addCollision(int id);
		void removeCollision(int id);
		int getCurrentCollision();
		bool collisionTargetPresent(int id);
		void move(forecastDirection directionParam);
		void incrementSpeed();
		void decrementSpeed();
		void updateDebugColors();

		void* Horse::operator new(size_t i);
};