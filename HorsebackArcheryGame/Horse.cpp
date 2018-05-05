#include "Horse.h"

//CONSTRUCTORS
Horse::Horse() {
	horse = NULL;
}

Horse::Horse(GLuint objectColorLocationParam, GLuint transformLocationParam, GLuint VAOParam, int drawTypeParam, int idParam)
{
	//Set all properties that need to be determined during runtime.
	objectColorLocation = objectColorLocationParam;
	transformLocation = transformLocationParam;
	VAO = VAOParam;
	drawType = drawTypeParam;
	id = idParam;
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
	collisionQueue = new Queue();

	pan = randomNumber(0, 72)*PI / 5;                    //Horse looks at random direction.
	scale = 0.8f + randomNumber(0, 22)*0.1f;             //Horse's size is varied by a reasonable range.
	speed = randomNumber(5, 20)*0.05f;
	currentStopFrame = 0;
	setStraightPathProperties();
	if (randomNumber(0, 1) == 0)
		direction = 1;
	else
		direction = -1;

	scaleOffset = 1 + scale;
	collisionRadius = 2.5f*(scaleOffset) / 2;

	if (speed >= CHANGE_ANIMATION_SPEED)
		setAnimationType(run);
	else
		setAnimationType(walk);

	randomizePosition();

	color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//Collision properties.
	overallStatus = normal;
	avoidingDirection = noDir;
	radiansTurnedInCollision = 0.0f;
	
	//Create body parts.
	horseTorso = new Node(color);
	horseNeck = new Node(color);
	horseHead = new Node(color);
	horseLeftUpperArm = new Node(color);
	horseRightUpperArm = new Node(color);
	horseLeftUpperLeg = new Node(color);
	horseRightUpperLeg = new Node(color);
	horseLeftLowerArm = new Node(color);
	horseRightLowerArm = new Node(color);
	horseLeftLowerLeg = new Node(color);
	horseRightLowerLeg = new Node(color);

	horse = new Tree(horseTorso, objectColorLocation, transformLocation, VAO, drawType);

	//Establish parent-child relationships between body parts of the horse.
	horseTorso->addChild(horseNeck);
	horseTorso->addChild(horseLeftUpperArm);
	horseTorso->addChild(horseRightUpperArm);
	horseTorso->addChild(horseLeftUpperLeg);
	horseTorso->addChild(horseRightUpperLeg);
	horseNeck->addChild(horseHead);
	horseLeftUpperArm->addChild(horseLeftLowerArm);
	horseRightUpperArm->addChild(horseRightLowerArm);
	horseLeftUpperLeg->addChild(horseLeftLowerLeg);
	horseRightUpperLeg->addChild(horseRightLowerLeg);

	//Initialize stacks used for scaling, and the stack for both rotation and translation.
	scaleMatrixStack = new Stack();
	rotTransMatrixStack = new Stack();
}

//PRIVATE FUNCTIONS
//Gets random integer from min to max
int Horse::randomNumber(int min, int max)
{
	return rand() % (max - min + 1) + min;
}

//Helps with rotation calculations where the center is different from the limb's center.
glm::mat4 Horse::rotateOffset(float x, float y, float z) {
	return glm::translate(modelMatrix, glm::vec3(x*scaleOffset, y*scaleOffset, z*scaleOffset));
}

void Horse::updateMatrices()
{
	horseTorsoScale = glm::scale(modelMatrix, glm::vec3(0.6f + scale*0.6, 0.2f + scale*0.2, 0.15f + scale*0.15));
	horseNeckScale = glm::scale(horseTorsoScale, glm::vec3(0.5f, 0.7f, 0.75f));
	horseHeadScale = glm::scale(horseNeckScale, glm::vec3(0.8f, 0.8f, 0.95f));
	horseLimbScale = glm::scale(horseTorsoScale, glm::vec3(0.1428f, 1.5f, 0.33f));

	horseTorsoRot = glm::translate(worldRotation, glm::vec3(0.0f + posX, 1.0f*scaleOffset, 0.0f + posZ))
		*glm::rotate(modelMatrix, pan, glm::vec3(0.0f, 1.0f, 0.0f));
	horseNeckRot = glm::translate(horseTorsoRot, glm::vec3(-0.75f*scaleOffset, 0.0f, 0.0f))
		*rotateOffset(0.3f, 0.0f, 0.0f)
		*glm::rotate(modelMatrix, -PI / 6 + jointAngles[1], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(-0.3f, 0.0f, 0.0f);
	horseHeadRot = glm::translate(horseNeckRot, glm::vec3(-0.4f*scaleOffset, 0.0f*scaleOffset, 0.0f))
		*rotateOffset(0.2f, 0.0f, 0.0f)
		*glm::rotate(modelMatrix, PI / 2 + jointAngles[0], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(-0.2f, 0.0f, 0.0f);
	horseLeftUpperArmRot = glm::translate(horseTorsoRot, glm::vec3(-0.45f*scaleOffset, -0.3f*scaleOffset, 0.1f*scaleOffset))
		*rotateOffset(0.0f, 0.25f, 0.0f)
		*glm::rotate(modelMatrix, jointAngles[7], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(0.0f, -0.25f, 0.0f);
	horseLeftLowerArmRot = glm::translate(horseLeftUpperArmRot, glm::vec3(0.0f, -0.4f*scaleOffset, 0.0f))
		*rotateOffset(0.0f, 0.2f, 0.0f)
		*glm::rotate(modelMatrix, jointAngles[6], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(0.0f, -0.2f, 0.0f);
	horseRightUpperArmRot = glm::translate(horseTorsoRot, glm::vec3(-0.45f*scaleOffset, -0.3f*scaleOffset, -0.1f*scaleOffset))
		*rotateOffset(0.0f, 0.25f, 0.0f)
		*glm::rotate(modelMatrix, jointAngles[3], glm::vec3(0.0f, 0.0f, 1.0f))*rotateOffset(0.0f, -0.25f, 0.0f);
	horseRightLowerArmRot = glm::translate(horseRightUpperArmRot, glm::vec3(0.0f, -0.4f*scaleOffset, 0.0f))
		*rotateOffset(0.0f, 0.2f, 0.0f)
		*glm::rotate(modelMatrix, jointAngles[2], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(0.0f, -0.2f, 0.0f);
	horseLeftUpperLegRot = glm::translate(horseTorsoRot, glm::vec3(0.45f*scaleOffset, -0.3f*scaleOffset, 0.1f*scaleOffset))
		*rotateOffset(0.0f, 0.25f, 0.0f)
		*glm::rotate(modelMatrix, jointAngles[9], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(0.0f, -0.25f, 0.0f);
	horseLeftLowerLegRot = glm::translate(horseLeftUpperLegRot, glm::vec3(0.0f, -0.4f*scaleOffset, 0.0f))
		*rotateOffset(0.0f, 0.2f, 0.0f)
		*glm::rotate(modelMatrix, jointAngles[8], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(0.0f, -0.2f, 0.0f);
	horseRightUpperLegRot = glm::translate(horseTorsoRot, glm::vec3(0.45f*scaleOffset, -0.3f*scaleOffset, -0.1f*scaleOffset))
		*rotateOffset(0.0f, 0.25f, 0.0f)
		*glm::rotate(modelMatrix, jointAngles[5], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(0.0f, -0.25f, 0.0f);
	horseRightLowerLegRot = glm::translate(horseRightUpperLegRot, glm::vec3(0.0f, -0.4f*scaleOffset, 0.0f))
		*rotateOffset(0.0f, 0.2f, 0.0f)
		*glm::rotate(modelMatrix, jointAngles[4], glm::vec3(0.0f, 0.0f, 1.0f))
		*rotateOffset(0.0f, -0.2f, 0.0f);

	horseTorso->setMatrices(horseTorsoScale, horseTorsoRot);
	horseNeck->setMatrices(horseNeckScale, horseNeckRot);
	horseHead->setMatrices(horseHeadScale, horseHeadRot);
	horseLeftUpperArm->setMatrices(horseLimbScale, horseLeftUpperArmRot);
	horseRightUpperArm->setMatrices(horseLimbScale, horseRightUpperArmRot);
	horseLeftUpperLeg->setMatrices(horseLimbScale, horseLeftUpperLegRot);
	horseRightUpperLeg->setMatrices(horseLimbScale, horseRightUpperLegRot);
	horseLeftLowerArm->setMatrices(horseLimbScale, horseLeftLowerArmRot);
	horseRightLowerArm->setMatrices(horseLimbScale, horseRightLowerArmRot);
	horseLeftLowerLeg->setMatrices(horseLimbScale, horseLeftLowerLegRot);
	horseRightLowerLeg->setMatrices(horseLimbScale, horseRightLowerLegRot);
}

void Horse::setColor(glm::vec4 &colorParam) {
	color = colorParam;
	horseTorso->setColor(color);
	horseNeck->setColor(color);
	horseHead->setColor(color);
	horseLeftUpperArm->setColor(color);
	horseRightUpperArm->setColor(color);
	horseLeftUpperLeg->setColor(color);
	horseRightUpperLeg->setColor(color);
	horseLeftLowerArm->setColor(color);
	horseRightLowerArm->setColor(color);
	horseLeftLowerLeg->setColor(color);
	horseRightLowerLeg->setColor(color);
}


//Change speed by a factor of up to 3 (i.e. up 3 times as much as the amount the speed increments).
//Animations change when the speed passes the threshold in either direction.
void Horse::randomSpeedChange() {
	float initialSpeed = speed;
	speed += randomNumber(-3, 3)*0.05f;
	if (speed > MAX_SPEED)
		speed = MAX_SPEED;
	else if (speed < MIN_SPEED)
		speed = MIN_SPEED;

	if (speed >= CHANGE_ANIMATION_SPEED && initialSpeed < CHANGE_ANIMATION_SPEED)
		setAnimationType(run);
	else if (speed < CHANGE_ANIMATION_SPEED && initialSpeed >= CHANGE_ANIMATION_SPEED)
		setAnimationType(walk);
}

//Go to next frame for stopping.
bool Horse::progressFrame() {
	if (currentStopFrame == stopFrames) {     //Stop period is over. Sets back the animation to walk or run depending on the horse speed.
		currentStopFrame = 0;
		isStopped = false;
		if (speed >= CHANGE_ANIMATION_SPEED)
			setAnimationType(run);
		else
			setAnimationType(walk);
		return false;
	}
	currentStopFrame++;
	return true;
}

//If a horse goes out of bounds next move, keep it near the edge.
void Horse::checkBounds() {
	if (posX < -50.0f)
		posX = -50.0f;
	else if (posX > 50.0f)
		posX = 50.0f;
	if (posZ < -50.0f)
		posZ = -50.0f;
	else if (posZ > 50.0f)
		posZ = 50.0f;
}

void Horse::executeAnimation() {
	if (animationType == walk)
		walkAnimation();
	else if (animationType == run)
		runAnimation();
	else
		jumpAnimation();
}

animation Horse::getAnimationType() {
	return animationType;
}

//Setup proper animation setup on animation change.
void Horse::setAnimationType(animation animationTypeParam) {
	if (animationTypeParam == walk)
		walkAnimationSetup();
	else if (animationTypeParam == run)
		runAnimationSetup();
	else
		jumpAnimationSetup();
	animationType = animationTypeParam;
}

//RUN ANIMATION
//Execute run animation for all body parts.
void Horse::runAnimation()
{
	runArmAnimation(2, 3);
	runArmAnimation(6, 7);
	runLegAnimation(4, 5);
	runLegAnimation(8, 9);
	runNeckAnimation(0, 1);
}

//Initial joint properties before run animation starts.
void Horse::runAnimationSetup()
{
	for (int i = 0; i < 10; i++) {
		jointAngles[i] = 0.0f;
		jointSpeed[i] = 1.0f;
		jointDirection[i] = 1.0f;
	}

	jointAngles[0] = -PI / 6;
	jointAngles[1] = -PI / 8;
	jointAngles[7] = -PI / 5;
	jointAngles[5] = PI / 3;
	jointAngles[9] = PI / 3 - PI / 5;
	jointSpeed[4] = 2.5f;
	jointSpeed[8] = 2.5f;
	jointDirection[4] = -1.0f;
	jointDirection[8] = -1.0f;
}

//Specific changes between angles, speed and direction to properly execute run animation for horse arms.
void Horse::runArmAnimation(int lowerLimb, int upperLimb)
{
	if (jointAngles[upperLimb] < -PI / 3) {
		jointDirection[upperLimb] = 1.0f;
		jointDirection[lowerLimb] = 1.0f;
		jointSpeed[lowerLimb] = 0.0f;
	}
	if (jointAngles[upperLimb] > PI / 18) {
		jointDirection[upperLimb] = -1.0f;
		jointDirection[lowerLimb] = -1.0f;
		jointAngles[lowerLimb] = PI / 2;
		jointSpeed[lowerLimb] = 1.1f;
	}

	if (jointAngles[upperLimb] < -PI / 36) {
		jointSpeed[upperLimb] = 1.0f;
	}
	else {
		jointSpeed[upperLimb] = 0.5f;
		if (jointDirection[lowerLimb] == 1.0f)
			jointSpeed[lowerLimb] = 5.0f;
	}

	if (jointAngles[lowerLimb] > PI / 2) {
		jointAngles[lowerLimb] = PI / 2;
		jointSpeed[lowerLimb] = 0.0f;
	}

	jointAngles[upperLimb] += ((RUN_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[upperLimb] * jointDirection[upperLimb]);
	jointAngles[lowerLimb] += ((RUN_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[lowerLimb] * jointDirection[lowerLimb]);
}

//Specific changes between angles, speed and direction to properly execute run animation for horse legs.
void Horse::runLegAnimation(int lowerLimb, int upperLimb)
{
	if (jointAngles[upperLimb] > PI / 3) {
		jointDirection[upperLimb] = -1.0f;
		jointDirection[lowerLimb] = -1.0f;
		jointSpeed[lowerLimb] = 2.5f;
	}

	if (jointAngles[upperLimb] < 0) {
		jointDirection[upperLimb] = 1.0f;
		jointDirection[lowerLimb] = 1.0f;
		jointSpeed[lowerLimb] = 2.5f;
	}

	if (jointAngles[upperLimb] < PI / 18) {
		jointSpeed[upperLimb] = 0.5f;
	}
	else
		jointSpeed[upperLimb] = 1.0f;

	if (jointAngles[lowerLimb] < -PI / 2) {
		jointAngles[lowerLimb] = -PI / 2;
		jointSpeed[lowerLimb] = 0.0f;
	}
	if (jointAngles[lowerLimb] > 0) {
		jointAngles[lowerLimb] = 0;
		jointSpeed[lowerLimb] = 0.0f;
	}

	jointAngles[upperLimb] += ((RUN_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[upperLimb] * jointDirection[upperLimb]);
	jointAngles[lowerLimb] += ((RUN_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[lowerLimb] * jointDirection[lowerLimb]);
}

//Specific changes between angles, speed and direction to properly execute run animation for horse's head and neck.
void Horse::runNeckAnimation(int head, int neck)
{
	if (jointAngles[neck] > PI / 30)
	{
		jointDirection[neck] = -1.0f;
		jointSpeed[neck] = 0.25f;
		jointDirection[head] = -1.0f;
		jointSpeed[head] = 0.25f;
	}
	if (jointAngles[neck] < -PI / 8)
	{
		jointDirection[neck] = 1.0f;
		jointSpeed[neck] = 0.5f;
		jointDirection[head] = 1.0f;
		jointSpeed[head] = 0.5f;
	}
	if (jointAngles[head] > 0.0f) {
		jointAngles[head] = 0.0f;
	}
	if (jointAngles[head] < -PI / 6) {
		jointAngles[head] = -PI / 6;
	}
	jointAngles[neck] += ((RUN_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[neck] * jointDirection[neck]);
	jointAngles[head] += ((RUN_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[head] * jointDirection[head]);
}

//WALK ANIMATION
//Execute walk animation for all body parts.
void Horse::walkAnimation() {
	walkArmAnimation(2, 3);
	walkArmAnimation(6, 7);
	walkLegAnimation(4, 5);
	walkLegAnimation(8, 9);
	walkNeckAnimation(0, 1);
}

//Initial joint properties before walk animation starts.
void Horse::walkAnimationSetup() {
	for (int i = 0; i < 10; i++) {
		jointAngles[i] = 0.0f;
		jointSpeed[i] = 1.0f;
		jointDirection[i] = 1.0f;
	}

	jointAngles[2] = PI / 36;
	jointDirection[2] = -1.0f;
	jointDirection[3] = 1.0f;
	jointSpeed[3] = 3.0f;
	jointAngles[6] = -PI / 6;
	jointDirection[6] = 1.0f;
	jointDirection[7] = -1.0f;
	jointSpeed[7] = 1.5f;
	jointAngles[4] = -PI / 36;
	jointDirection[4] = 1.0f;
	jointDirection[5] = -1.0f;
	jointSpeed[5] = 1.5f;
	jointAngles[8] = PI / 6;
	jointDirection[8] = -1.0f;
	jointDirection[9] = 1.0f;
	jointSpeed[9] = 3.0f;
}

//Specific changes between angles, speed and direction to properly execute walk animation for horse arms.
void Horse::walkArmAnimation(int lowerLimb, int upperLimb) {
	if (jointAngles[upperLimb] < -PI / 6) {
		jointDirection[upperLimb] = 1.0f;
		jointDirection[lowerLimb] = -1.0f;
		jointSpeed[lowerLimb] = 1.5f;
	}
	if (jointAngles[upperLimb] > PI / 36) {
		jointDirection[upperLimb] = -1.0f;
		jointDirection[lowerLimb] = 1.0f;
		jointSpeed[lowerLimb] = 3.0f;
	}

	if (jointAngles[upperLimb] < 0)
		jointSpeed[upperLimb] = 1.0f;
	else
		jointSpeed[upperLimb] = 0.2f;

	if (jointAngles[lowerLimb] > PI / 4) {
		jointAngles[lowerLimb] = PI / 4;
		jointSpeed[lowerLimb] = 0.0f;
	}
	if (jointAngles[lowerLimb] < 0) {
		jointAngles[lowerLimb] = 0.0f;
		jointSpeed[lowerLimb] = 0.0f;
	}

	jointAngles[upperLimb] += ((WALK_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[upperLimb] * jointDirection[upperLimb]);
	jointAngles[lowerLimb] += ((WALK_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[lowerLimb] * jointDirection[lowerLimb]);
}

//Specific changes between angles, speed and direction to properly execute walk animation for horse legs.
void Horse::walkLegAnimation(int lowerLimb, int upperLimb) {
	if (jointAngles[upperLimb] < -PI / 36) {
		jointDirection[upperLimb] = 1.0f;
		jointDirection[lowerLimb] = -1.0f;
		jointSpeed[lowerLimb] = 1.5f;
	}
	if (jointAngles[upperLimb] > PI / 6) {
		jointDirection[upperLimb] = -1.0f;
		jointDirection[lowerLimb] = 1.0f;
		jointSpeed[lowerLimb] = 3.0f;
	}

	if (jointAngles[upperLimb] > 0)
		jointSpeed[upperLimb] = 1.0f;
	else
		jointSpeed[upperLimb] = 0.2f;

	if (jointAngles[lowerLimb] > PI / 4) {
		jointAngles[lowerLimb] = PI / 4;
		jointSpeed[lowerLimb] = 0.0f;
	}
	if (jointAngles[lowerLimb] < 0) {
		jointAngles[lowerLimb] = 0.0f;
		jointSpeed[lowerLimb] = 0.0f;
	}

	jointAngles[upperLimb] += ((WALK_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[upperLimb] * jointDirection[upperLimb]);
	jointAngles[lowerLimb] += ((WALK_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[lowerLimb] * jointDirection[lowerLimb]);
}

//Specific changes between angles, speed and direction to properly execute walk animation for horse's head and neck.
void Horse::walkNeckAnimation(int head, int neck) {
	if (jointAngles[neck] > PI / 45)
	{
		jointDirection[neck] = -1.0f;
		jointSpeed[neck] = 0.25f;
		jointDirection[head] = -1.0f;
		jointSpeed[head] = 0.25f;
	}
	if (jointAngles[neck] < -PI / 30)
	{
		jointDirection[neck] = 1.0f;
		jointSpeed[neck] = 0.5f;
		jointDirection[head] = 1.0f;
		jointSpeed[head] = 0.5f;
	}
	if (jointAngles[head] > 0.0f) {
		jointAngles[head] = 0.0f;
	}
	if (jointAngles[head] < -PI / 6) {
		jointAngles[head] = -PI / 6;
	}
	jointAngles[neck] += ((WALK_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[neck] * jointDirection[neck]);
	jointAngles[head] += ((WALK_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[head] * jointDirection[head]);
}

//JUMP ANIMATION
//Execute jump animation for all body parts.
void Horse::jumpAnimation() {
	jumpArmAnimation(2, 3);
	jumpArmAnimation(6, 7);
	jumpLegAnimation(4, 5);
	jumpLegAnimation(8, 9);
	jumpNeckAnimation(0, 1);
}

//Initial joint properties before jump animation starts.
void Horse::jumpAnimationSetup() {
	for (int i = 0; i < 10; i++) {
		jointAngles[i] = 0.0f;
		jointSpeed[i] = 1.0f;
		jointDirection[i] = 1.0f;
	}

	jointSpeed[5] = 0.0f;
	jointDirection[5] = -1.0f;
	jointSpeed[9] = 0.0f;
	jointDirection[9] = -1.0f;
}

//Specific changes between angles, speed and direction to properly execute jump animation for horse arms.
void Horse::jumpArmAnimation(int lowerLimb, int upperLimb) {
	if (jointAngles[upperLimb] < -PI / 3) {
		jointDirection[upperLimb] = 1.0f;
		jointDirection[lowerLimb] = -1.0f;
	}
	if (jointAngles[upperLimb] > 0) {
		jointDirection[upperLimb] = -1.0f;
		jointDirection[lowerLimb] = 1.0f;
		jointSpeed[lowerLimb] = 2.0f;
	}

	if (jointAngles[upperLimb] > -PI / 4) {
		jointSpeed[upperLimb] = 1.5f;
		if (jointDirection[lowerLimb] == -1.0f && jointAngles[lowerLimb] > 0.0f)
			jointSpeed[lowerLimb] = 5.0f;
	}
	else
		jointSpeed[upperLimb] = 0.2f;

	if (jointAngles[lowerLimb] > 5* PI / 8) {
		jointAngles[lowerLimb] = 5 * PI / 8;
		jointSpeed[lowerLimb] = 0.0f;
	}
	if (jointAngles[lowerLimb] < 0) {
		jointAngles[lowerLimb] = 0.0f;
		jointSpeed[lowerLimb] = 0.0f;
	}

	jointAngles[upperLimb] += ((JUMP_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[upperLimb] * jointDirection[upperLimb]);
	jointAngles[lowerLimb] += ((JUMP_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[lowerLimb] * jointDirection[lowerLimb]);
}

//Specific changes between angles, speed and direction to properly execute jump animation for horse legs.
void Horse::jumpLegAnimation(int lowerLimb, int upperLimb) {
	if (jointAngles[upperLimb] < 0) {
		jointDirection[upperLimb] = 1.0f;
		jointDirection[lowerLimb] = 1.0f;
	}
	if (jointAngles[upperLimb] > PI / 3) {
		jointDirection[upperLimb] = -1.0f;
		jointDirection[lowerLimb] = -1.0f;
		jointSpeed[lowerLimb] = 2.5f;
	}

	if (jointAngles[upperLimb] < PI / 4) {
		jointSpeed[upperLimb] = 1.5f;
		if (jointDirection[upperLimb] == 1.0f)
			jointSpeed[lowerLimb] = 0.0f;
	}
	else
		jointSpeed[upperLimb] = 0.2f;

	if (jointAngles[lowerLimb] < -5 * PI / 8) {
		jointAngles[lowerLimb] = -5 * PI / 8;
		jointDirection[lowerLimb] = 1.0f;
	}
	if (jointAngles[lowerLimb] > 0) {
		jointAngles[lowerLimb] = 0.0f;
		jointSpeed[lowerLimb] = 0.0f;
	}

	jointAngles[upperLimb] += ((JUMP_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[upperLimb] * jointDirection[upperLimb]);
	jointAngles[lowerLimb] += ((JUMP_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[lowerLimb] * jointDirection[lowerLimb]);
}

//Specific changes between angles, speed and direction to properly execute jump animation for horse's head and neck.
void Horse::jumpNeckAnimation(int head, int neck) {
	if (jointAngles[neck] > PI / 15)
	{
		jointDirection[neck] = -1.0f;
		jointSpeed[neck] = 0.125f;
		jointDirection[head] = -1.0f;
		jointSpeed[head] = 0.125f;
	}
	if (jointAngles[neck] < -PI / 40)
	{
		jointDirection[neck] = 1.0f;
		jointSpeed[neck] = 0.25f;
		jointDirection[head] = 1.0f;
		jointSpeed[head] = 0.25f;
	}
	if (jointAngles[head] > 0.0f) {
		jointAngles[head] = 0.0f;
	}
	if (jointAngles[head] < -PI / 6) {
		jointAngles[head] = -PI / 6;
	}
	jointAngles[neck] += ((JUMP_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[neck] * jointDirection[neck]);
	jointAngles[head] += ((JUMP_SPEED_MULTIPLIER*PI / 180.0f)*jointSpeed[head] * jointDirection[head]);
}

//PUBLIC FUNCTIONS

//FUNCTIONS RELATED TO DRAWING THE HORSE ITSELF
void Horse::draw() {
	horse->drawTraversalFromRoot(scaleMatrixStack, rotTransMatrixStack);
	updateMatrices();
}

void Horse::updatePosition() 
{
	//All scenarios when horse moves (controlled horse moves using different functions)
	if ((getCollisionStatus() != stopped && getCollisionStatus() != controlled) && !isStopped) {
		executeAnimation(); //Always execute animation when movement occurs.

		//Go straight if it doesn't collide with any other horse.
		if (avoidingDirection == straightDir || (avoidingDirection == noDir && getCollisionStatus() != avoiding)) {
			if (currentSteps == stepToChangeSpeedAt && doWeChangeSpeed) //Change speed at proper step if applicable.
				randomSpeedChange();
			if (currentSteps == stepToChangeSpeedAt && doWeStop)        //Stop at proper step if applicable.
				stopHorse();
			if ((currentSteps < steps || getCollisionStatus() != normal) && (posX >= -50.0f && posX <= 50.0f) && (posZ >= -50.0f && posZ <= 50.0f)) {
				radiansTurnedInCollision = 0.0f;
				if (getCollisionStatus() == normal)
					currentSteps++;
				posX += speed*cos(pan + PI);
				posZ += speed*-sin(pan + PI);
				checkBounds();
			}
			else
			{
				checkBounds();
				setStraightPathProperties();
				pan += DEGREES_TO_TURN * direction;
				if (randomNumber(0, 1) == 0)
					direction = 1;
				else
					direction = -1;
			}
		}
		//Turn left
		else if (avoidingDirection == leftDir) {
			pan += DEGREES_TO_TURN;
			radiansTurnedInCollision += DEGREES_TO_TURN;
		}
		//Turn right
		else {
			pan -= DEGREES_TO_TURN;
			radiansTurnedInCollision += DEGREES_TO_TURN;
		}
	}

	//If horse is selected to randomly stop, progress frames until horse can move again.
	else if (isStopped) {
		if (currentStopFrame <= JUMP_FRAMES) //Do animation for horse only once!
			executeAnimation();
		progressFrame();
	}
}

//GETTERS
glm::vec3 Horse::getPosition() {
	return glm::vec3(posX, posY, posZ);
}

float Horse::getCollisionRadius() {
	return collisionRadius;
}

status Horse::getCollisionStatus() {
	return overallStatus;
}

int Horse::getId() {
	return id;
}

forecastDirection Horse::getAvoidingDirection() {
	return avoidingDirection;
}

bool Horse::getDirectionAssigned() {
	return directionAssigned;
}

bool Horse::getIsHorseStopped() {
	return isStopped;
}

//Predict where horse is going to be when going straight.
glm::vec3 Horse::getForecastedPosition(forecastDirection direction) {
	if (direction == straightDir) {
		float forecastedPosX = posX + speed*cos(pan + PI);
		float forecastedPosZ = posZ + speed*-sin(pan + PI);
		return glm::vec3(forecastedPosX, posY, forecastedPosZ);
	}
	else
		return glm::vec3(posX, posY, posZ);
}

//SETTERS
void Horse::setCollisionStatus(status statusParam) {
	if (statusParam == controlled || !isControlled)
		overallStatus = statusParam;
	if (debugCollisionStatus && !isControlled && !isSelected) {
		if (statusParam == normal)
			setColor(WHITE);
		else if (statusParam == stopped)
			setColor(YELLOW);
		else
			setColor(BLUE);
	}
}


void Horse::setWorldRotation(glm::mat4 &worldRotationParam) {
	worldRotation = worldRotationParam;
}

void Horse::setDrawType(int drawTypeParam) {
	drawType = drawTypeParam;
	horse->setDrawType(drawTypeParam);
}

void Horse::setAvoidingDirection(forecastDirection direction){
	avoidingDirection = direction;
}

void Horse::setDirectionAssigned(bool directionAssignedParam) {
	directionAssigned = directionAssignedParam;
}

void Horse::setIsSelected(bool isSelectedParam) {
	if (isSelectedParam)
		setColor(FUCHSIA);
	else if (!isControlled)
		setColor(WHITE);
	isSelected = isSelectedParam;
}

void Horse::setIsControlled(bool isControlledParam) {
	if (isControlledParam)
		setColor(PURPLE);
	else
		setColor(WHITE);
	setCollisionStatus(controlled);
	isControlled = isControlledParam;
}

//Horses are certain colors depending on collision status when debugging collisions.
void Horse::setDebugCollisionStatus(bool statusParam)
{
	debugCollisionStatus = statusParam;
}

//FUNCTIONS RELATED TO OTHER HORSE PROPERTIES.
void Horse::randomizePosition() {
	posX = (float)randomNumber(-50, 50);
	posY = 1.0f*scaleOffset;
	posZ = (float)randomNumber(-50, 50);
}

//Resets properties that are only relevant when a horse goes a straight path.
void Horse::setStraightPathProperties()
{
	currentSteps = 0;
	steps = randomNumber(10, 30);
	stepToChangeSpeedAt = randomNumber(0, steps);
	stepToStopAt = randomNumber(0, steps);
	doWeChangeSpeed = randomNumber(0, 9) < 5 ? true : false;
	doWeStop = randomNumber(0, 9) < 2 ? true : false;
}

bool Horse::doCollisionsExist() {
	if (collisionQueue->getSize() > 0)
		return true;
	else
		return false;
}

//Horse is trapped if turned 360 degrees cumulatively (i.e. not necessarily 360 degrees left or right entirely)
bool Horse::isTrapped() {
	if (radiansTurnedInCollision >= 2 * PI) {
		radiansTurnedInCollision = 0.0f;
		return true;
	}
	else
		return false;
}

//Horse jumps once when stopped.
void Horse::stopHorse() {
	//If horse is controlled, we wait until one jump is complete (so the user doesn't experience unnecessary wait time).
	if (getCollisionStatus() == controlled)
		stopFrames = JUMP_FRAMES;
	else
		stopFrames = randomNumber(JUMP_FRAMES, JUMP_FRAMES * 2);
	setAnimationType(jump);
	isStopped = true;
}

void Horse::addCollision(int id) {
	collisionQueue->enqueue(id);
}

void Horse::removeCollision(int id) {
	collisionQueue->removeElement(id);
}

int Horse::getCurrentCollision() {
	return collisionQueue->getFront();
}

//Check if a horse is in the list of collisions.
bool Horse::collisionTargetPresent(int id) {
	bool identified = false;
	for (int i = 0; i < collisionQueue->getSize(); i++) {
		if (collisionQueue->getElement(i) == id) {
			identified = true;
			break;
		}
	}
	return identified;
}

//Movement function for controlled horse.
void Horse::move(forecastDirection direction) {
	if (!isStopped) {
		if (direction == straightDir) {
			posX += speed*cos(pan + PI);
			posZ += speed*-sin(pan + PI);
			checkBounds();
			executeAnimation(); //Controlled horse only animates when moving (only jumps when randomly selected to stop).
		}
		else if (direction == leftDir)
			pan += DEGREES_TO_TURN;
		else if (direction == rightDir)
			pan -= DEGREES_TO_TURN;
	}
}

void Horse::incrementSpeed() {
	speed += 0.05f;
	if (speed > MAX_SPEED)
		speed = MAX_SPEED;
	if (speed >= CHANGE_ANIMATION_SPEED)
		setAnimationType(run);
}

void Horse::decrementSpeed() {
	speed -= 0.05f;
	if (speed < MIN_SPEED)
		speed = MIN_SPEED;
	if (speed < CHANGE_ANIMATION_SPEED)
		setAnimationType(walk);
}

//If horse isn't controlled or selected, 
//WHITE indicates no collision, 
//YELLOW indicates stopping during collision, 
//BLUE indicates avoiding during collision
void Horse::updateDebugColors() {
	if (!isControlled && !isSelected) {
		if (debugCollisionStatus) {
			if (getCollisionStatus() == normal)
				setColor(WHITE);
			else if (getCollisionStatus() == stopped)
				setColor(YELLOW);
			else
				setColor(BLUE);
		}
		else
			setColor(WHITE);
	}
}

//Allows new instances of node to have the proper byte boundaries (they change since glm::mat4 parameters are passed)!
void* Horse::operator new(size_t i)
{
	return _mm_malloc(i, 16);
}