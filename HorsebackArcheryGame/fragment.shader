#version 330 core

out vec4 color;

uniform vec4 objectColor;
uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPosition;    //Influences specular lighting.
uniform bool shadowsActive;

in vec4 colorPosition;
in vec2 textureCoordinate;
in vec3 normalCoordinate;
in vec4 colorPositionInLight; //For shadow calculations

uniform sampler2D textureContent;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 colorPositionInLight)
{
	vec3 projectionCoordinates = colorPositionInLight.xyz/colorPositionInLight.w; //Allows proper shadow calculations for perspective lighting
	projectionCoordinates = projectionCoordinates * 0.5 + 0.5;                    //Convert to [0,1] via texture coordinate format.
	float closestDepth = texture(shadowMap, projectionCoordinates.xy).r;          //Grab depth of shadow map.
	float currentDepth = projectionCoordinates.z;                                 //Grab actual depth of pixel.
	float shadow;
	if (currentDepth > closestDepth && shadowsActive)                             //We have a shadow if the actual depth is below the depth of the shadow map!
		shadow = 1.0;
	else
		shadow = 0.0;

	return shadow;
}

//Used solely for debugging the first pass depth map with perspective shadows.
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * 1.0 * 25.0) / (25.0 + 1.0 - z * (25.0 - 1.0));
}

void main()
{
	//Calculates ambient lighting (i.e the brightness of the scene with no light source present).
	float ambientStrength = 0.3;                        //How bright it is when no light source is present.
	vec4 ambient = ambientStrength * lightColor;
	
	//Calculates diffuse lighting (i.e. allows color to change when face of object faces towards or away from the light).
	vec3 normal = normalize(normalCoordinate);
	vec3 lightDirection = normalize(lightPosition-vec3(colorPosition.x, colorPosition.y, colorPosition.z));
	float diffuseCoefficient = max(dot(normal, lightDirection), 0.0); //Don't allow negative values for light related calculations.
	vec4 diffuse = diffuseCoefficient * lightColor;
	
	//Calculates specular lighting (i.e. allows shiny to spot to appear when observing a spot that the light directly shines on)
	float specularStrength = 0.5;
	vec3 viewDirection = normalize(viewPosition - vec3(colorPosition.x, colorPosition.y, colorPosition.z)); 
	vec3 reflectDirection = reflect(-lightDirection, normal);
	float specularCoefficient = pow(max(dot(viewDirection, reflectDirection), 0.0), 512);  //Don't allow negative values for light related calculations.
	                                                                                       //The higher the second value, it becomes more shiny but has less area.
																						   //Recommended to set this value in a form of 2^n
	vec4 specular = specularStrength * specularCoefficient * lightColor;
	
	//Final color calculation. Shadow only influences diffuse and specular so shadows aren't complete darkness.
	float shadow = ShadowCalculation(colorPositionInLight);
	vec4 finalColor = (ambient + (1.0-shadow) * (diffuse + specular)) * objectColor;
	//vec4 finalColor = (ambient + diffuse + specular) * objectColor;
    color = texture(textureContent, textureCoordinate) * finalColor;
	
	//Allows us to debug the shadow map during first pass.
	//float depthValue = texture(shadowMap, textureCoordinate).r;
	//color = vec4(vec3(LinearizeDepth(depthValue) / 25.0), 1.0);   //Debug for Perspective shadows.
	//color = vec4(vec3(depthValue), 1.0)                           //Debug for Orthogonal shadows.
} 