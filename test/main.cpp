/**
 * @file    main.cpp
 * @brief   End-to-end tests for the graphics engine
 *
 * @author  Group 10 (C) SUPSI
 */

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdio.h>

#include "engine.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Test counters
static int totalTests = 0;
static int passedTests = 0;

#define TEST(name)                                 \
	std::cout << "\n[TEST] " << name << std::endl; \
	totalTests++;

#define TEST_PASS() \
	passedTests++;  \
	std::cout << "  ✓ PASSED" << std::endl;

#define TEST_FAIL(msg) \
	std::cout << "  ✗ FAILED: " << msg << std::endl;

// Helper functions
bool floatEqual(float a, float b, float epsilon = 0.0001f)
{
	return std::fabs(a - b) < epsilon;
}

bool vec3Equal(const glm::vec3 &a, const glm::vec3 &b, float epsilon = 0.0001f)
{
	return floatEqual(a.x, b.x, epsilon) &&
		   floatEqual(a.y, b.y, epsilon) &&
		   floatEqual(a.z, b.z, epsilon);
}

bool mat4Equal(const glm::mat4 &a, const glm::mat4 &b, float epsilon = 0.0001f)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (!floatEqual(a[i][j], b[i][j], epsilon))
				return false;
		}
	}
	return true;
}

// ============================================================================
// OBJECT TESTS
// ============================================================================

void testObjectCreation()
{
	TEST("Object creation and ID assignment");

	Eng::Node *node1 = new Eng::Node("Node1");
	Eng::Node *node2 = new Eng::Node("Node2");

	assert(node1->getName() == "Node1");
	assert(node2->getName() == "Node2");
	assert(node1->getId() != node2->getId());

	delete node1;
	delete node2;

	TEST_PASS();
}

// ============================================================================
// NODE TESTS
// ============================================================================

void testNodeHierarchy()
{
	TEST("Node parent-child hierarchy");

	Eng::Node *root = new Eng::Node("Root");
	Eng::Node *child1 = new Eng::Node("Child1");
	Eng::Node *child2 = new Eng::Node("Child2");

	// Test adding children
	assert(root->addChild(child1) == true);
	assert(root->addChild(child2) == true);
	assert(root->getNumberOfChildren() == 2);

	// Test parent assignment
	assert(child1->getParent() == root);
	assert(child2->getParent() == root);

	// Test duplicate child (should fail)
	assert(root->addChild(child1) == false);

	// Test get child by index
	assert(root->getChild(0) == child1);
	assert(root->getChild(1) == child2);
	assert(root->getChild(2) == nullptr);

	// Test get child by name
	assert(root->getChild("Child1") == child1);
	assert(root->getChild("Child2") == child2);
	assert(root->getChild("NonExistent") == nullptr);

	delete root; // Should cascade delete children

	TEST_PASS();
}

void testNodeTransformation()
{
	TEST("Node transformation matrix");

	Eng::Node *node = new Eng::Node("TestNode");

	// Test identity matrix
	glm::mat4 identity = glm::mat4(1.0f);
	assert(mat4Equal(node->getMatrix(), identity));

	// Test translation
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 10.0f, 15.0f));
	node->setMatrix(translation);
	assert(mat4Equal(node->getMatrix(), translation));

	// Test world coordinates (no parent)
	glm::mat4 world = node->getWorldCoordinateMatrix();
	assert(mat4Equal(world, node->getMatrix())); // No parent = getMarix

	delete node;

	TEST_PASS();
}

void testNodeWorldCoordinates()
{
	TEST("Node world coordinate transformation");

	Eng::Node *root = new Eng::Node("ROORORO");
	Eng::Node *parent = new Eng::Node("Parent");
	Eng::Node *child = new Eng::Node("Child");

	// Set parent transformation
	glm::mat4 parentTrans = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f));
	parent->setMatrix(parentTrans);

	// Set child transformation
	glm::mat4 childTrans = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
	child->setMatrix(childTrans);

	// Add child to parent
	root->addChild(parent);
	parent->addChild(child);
	// Test world coordinates
	glm::mat4 childWorld = child->getWorldCoordinateMatrix();
	glm::vec3 worldPos = glm::vec3(childWorld[3]);

	// Child shoudld be at parent position + child local position
	assert(vec3Equal(worldPos, glm::vec3(15.0f, 0.0f, 0.0f)));

	delete parent;

	TEST_PASS();
}

void testNodeMovement()
{
	TEST("Node animation movement");

	Eng::Node *node = new Eng::Node("AnimNode");

	// Initial position
	glm::mat4 initial = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	node->setMatrix(initial);

	// Setup movement
	glm::mat4 step = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	node->move(step, 10);

	assert(node->isMoving() == true);

	// Execute some steps
	for (int i = 0; i < 5; i++)
	{
		node->calculateMove();
	}

	assert(node->isMoving() == true); // Still moving

	// Execute remaining steps
	for (int i = 0; i < 5; i++)
	{
		node->calculateMove();
	}

	assert(node->isMoving() == false); // Movement complete

	// Reset movement
	node->resetMove();
	assert(node->isMoving() == false);

	delete node;

	TEST_PASS();
}

void testNodeRemoval()
{
	TEST("Node child removal");

	Eng::Node *parent = new Eng::Node("Parent");
	Eng::Node *child1 = new Eng::Node("Child1");
	Eng::Node *child2 = new Eng::Node("Child2");
	Eng::Node *child3 = new Eng::Node("Child3");

	parent->addChild(child1);
	parent->addChild(child2);
	parent->addChild(child3);

	assert(parent->getNumberOfChildren() == 3);

	// Remove by index
	assert(parent->removeChild(1) == true);
	assert(parent->getNumberOfChildren() == 2);
	assert(parent->getChild(0) == child1);
	assert(parent->getChild(1) == child3);

	// Remove by pointer
	assert(parent->removeChild(child3) == true);
	assert(parent->getNumberOfChildren() == 1);

	// Remove non-existent
	assert(parent->removeChild(child2) == false);

	delete parent;
	delete child2; // Not deleted by parent

	TEST_PASS();
}

// ============================================================================
// CAMERA TESTS
// ============================================================================

void testPerspectiveCamera()
{
	TEST("Perspective camera configuration");

	glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 10.0f));
	Eng::PerspectiveCamera *camera = new Eng::PerspectiveCamera("PerspCam", pos);

	// Set camera parameters
	float fov = 45.0f;
	float ratio = 16.0f / 9.0f;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	camera->setCameraParams(fov, ratio, nearPlane, farPlane);

	assert(floatEqual(camera->getFOV(), fov));
	assert(floatEqual(camera->getRatio(), ratio));
	assert(floatEqual(camera->getNearPlane(), nearPlane));
	assert(floatEqual(camera->getFarPlane(), farPlane));

	// Test projection matrix exists
	glm::mat4 proj = camera->getProjectionMatrix();
	assert(proj != glm::mat4(0.0f));

	// Test view matrix
	glm::mat4 view = camera->getViewMatrix();
	assert(view != glm::mat4(0.0f));

	delete camera;

	TEST_PASS();
}

void testOrthoCamera()
{
	TEST("Orthographic camera configuration");

	glm::mat4 pos = glm::mat4(1.0f);
	Eng::OrthoCamera *camera = new Eng::OrthoCamera("OrthoCam", pos);

	// Set camera parameters (width/height version)
	camera->setCameraParams(800, 600, 0.1f, 1000.0f);

	assert(camera->getLeft() == -400.0f);
	assert(camera->getRight() == 400.0f);
	assert(camera->getBottom() == -300.0f);
	assert(camera->getTop() == 300.0f);
	assert(floatEqual(camera->getNearPlane(), 0.1f));
	assert(floatEqual(camera->getFarPlane(), 1000.0f));

	// Set camera parameters (explicit bounds version)
	camera->setCameraParams(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 100.0f);

	assert(floatEqual(camera->getLeft(), -10.0f));
	assert(floatEqual(camera->getRight(), 10.0f));
	assert(floatEqual(camera->getBottom(), -10.0f));
	assert(floatEqual(camera->getTop(), 10.0f));

	delete camera;

	TEST_PASS();
}

// ============================================================================
// MATERIAL TESTS
// ============================================================================

void testMaterial()
{
	TEST("Material properties");

	glm::vec4 emission(0.1f, 0.1f, 0.1f, 1.0f);
	glm::vec4 ambient(0.2f, 0.2f, 0.2f, 1.0f);
	glm::vec4 diffuse(0.8f, 0.8f, 0.8f, 1.0f);
	glm::vec4 specular(0.5f, 0.5f, 0.5f, 1.0f);
	float shininess = 64.0f;

	Eng::Material *material = new Eng::Material(
		"TestMaterial", emission, ambient, diffuse, specular, shininess);

	assert(material->getName() == "TestMaterial");
	assert(material->getEmission() == emission);
	assert(material->getAmbient() == ambient);
	assert(material->getDiffuse() == diffuse);
	assert(material->getSpecular() == specular);
	assert(floatEqual(material->getShininess(), shininess));

	// Test setters
	glm::vec4 newAmbient(0.3f, 0.3f, 0.3f, 1.0f);
	material->setAmbient(newAmbient);
	assert(material->getAmbient() == newAmbient);

	delete material;

	TEST_PASS();
}

// ============================================================================
// MESH TESTS
// ============================================================================

void testMeshCreation()
{
	TEST("Mesh creation with geometry");

	// Create simple triangle mesh
	std::vector<glm::vec3> vertices = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.5f, 1.0f, 0.0f)};

	std::vector<glm::uvec3> faces = {
		glm::uvec3(0, 1, 2)};

	std::vector<glm::vec4> normals = {
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)};

	std::vector<glm::vec2> texCoords = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.5f, 1.0f)};

	Eng::Mesh *mesh = new Eng::Mesh(
		"TriangleMesh",
		glm::mat4(1.0f),
		vertices,
		faces,
		normals,
		texCoords);

	assert(mesh->getName() == "TriangleMesh");
	assert(mesh->getMaterial() == nullptr); // No material set yet

	// Create and assign material
	Eng::Material *material = new Eng::Material("MeshMaterial");
	mesh->setMaterial(material);
	assert(mesh->getMaterial() == material);

	delete mesh;
	delete material;

	TEST_PASS();
}

// ============================================================================
// LIGHT TESTS
// ============================================================================

void testOmniLight()
{
	TEST("Omni light creation and properties");

	glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
	Eng::OmniLight *light = new Eng::OmniLight("OmniLight1", pos);

	// Test default values
	assert(light->getName() == "OmniLight1");

	// Test color properties
	glm::vec4 color(1.0f, 0.8f, 0.6f, 1.0f);
	light->setAmbient(color);
	light->setDiffuse(color);
	light->setSpecular(color);

	assert(light->getAmbient() == color);
	assert(light->getDiffuse() == color);
	assert(light->getSpecular() == color);

	// Test attenuation
	light->setAttenuaton(1.0f, 0.1f, 0.01f);

	delete light;

	TEST_PASS();
}

void testSpotLight()
{
	TEST("Spot light with cutoff angle");

	glm::mat4 pos = glm::mat4(1.0f);
	float cutoff = 30.0f;
	Eng::SpotLight *light = new Eng::SpotLight("SpotLight1", pos, cutoff);

	assert(light->getName() == "SpotLight1");
	assert(floatEqual(light->getCutoff(), cutoff));

	// Test cutoff modification
	light->setCutoff(45.0f);
	assert(floatEqual(light->getCutoff(), 45.0f));

	// Test invalid cutoff (negative)
	light->setCutoff(-10.0f);
	assert(floatEqual(light->getCutoff(), 0.0f));

	delete light;

	TEST_PASS();
}

void testInfiniteLight()
{
	TEST("Infinite (directional) light");

	glm::mat4 dir = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Eng::InfiniteLight *light = new Eng::InfiniteLight("DirLight", dir);

	assert(light->getName() == "DirLight");

	glm::vec4 color(0.9f, 0.9f, 1.0f, 1.0f);
	light->setDiffuse(color);
	assert(light->getDiffuse() == color);

	delete light;

	TEST_PASS();
}

// ============================================================================
// LIST TESTS
// ============================================================================

void testListManagement()
{
	TEST("Scene list management");

	Eng::List *list = new Eng::List("SceneList");

	// Create test scene
	Eng::Node *root = new Eng::Node("Root");
	Eng::Mesh *mesh1 = new Eng::Mesh("Mesh1");
	Eng::OmniLight *light1 = new Eng::OmniLight("Light1");

	root->addChild(mesh1);
	root->addChild(light1);

	// Create and set camera
	Eng::PerspectiveCamera *camera = new Eng::PerspectiveCamera("Camera");
	camera->setCameraParams(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	list->setCamera(camera);

	// Pass through scene
	list->pass(root, glm::mat4(1.0f));

	// Clear list
	list->clear();

	delete list;
	delete root;
	delete camera;

	TEST_PASS();
}

// ============================================================================
// COMPLEX INTEGRATION TESTS
// ============================================================================

void testComplexSceneGraph()
{
	TEST("Complex scene graph hierarchy");

	// Create multi-level hierarchy
	Eng::Node *root = new Eng::Node("SceneRoot");
	Eng::Node *environment = new Eng::Node("Environment");
	Eng::Node *characters = new Eng::Node("Characters");

	Eng::Mesh *ground = new Eng::Mesh("Ground");
	Eng::Mesh *sky = new Eng::Mesh("Sky");
	Eng::Mesh *player = new Eng::Mesh("Player");
	Eng::Mesh *enemy = new Eng::Mesh("Enemy");

	Eng::OmniLight *sunLight = new Eng::OmniLight("Sun");
	Eng::SpotLight *torchLight = new Eng::SpotLight("Torch");

	// Build hierarchy
	root->addChild(environment);
	root->addChild(characters);

	environment->addChild(ground);
	environment->addChild(sky);
	environment->addChild(sunLight);

	characters->addChild(player);
	characters->addChild(enemy);
	player->addChild(torchLight);

	// Verify structure
	assert(root->getNumberOfChildren() == 2);
	assert(environment->getNumberOfChildren() == 3);
	assert(characters->getNumberOfChildren() == 2);
	assert(player->getNumberOfChildren() == 1);

	// Test recursive search
	assert(root->getChild("Ground") == ground);
	assert(root->getChild("Player") == player);
	assert(root->getChild("Torch") == torchLight);

	// Test transformations propagation
	glm::mat4 envTrans = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 0.0f));
	environment->setMatrix(envTrans);

	glm::mat4 groundTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, 0.0f));
	ground->setMatrix(groundTrans);

	glm::mat4 groundWorld = ground->getWorldCoordinateMatrix();
	glm::vec3 groundWorldPos = glm::vec3(groundWorld[3]);
	assert(vec3Equal(groundWorldPos, glm::vec3(100.0f, -10.0f, 0.0f)));

	delete root; // Cascades to all children

	TEST_PASS();
}

void testAnimatedHierarchy()
{
	TEST("Animated node hierarchy");

	Eng::Node *armature = new Eng::Node("Armature");
	Eng::Node *upperArm = new Eng::Node("UpperArm");
	Eng::Node *foreArm = new Eng::Node("ForeArm");
	Eng::Node *hand = new Eng::Node("Hand");

	// Build arm hierarchy
	armature->addChild(upperArm);
	upperArm->addChild(foreArm);
	foreArm->addChild(hand);

	// Set initial positions
	glm::mat4 upperPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f));
	glm::mat4 forePos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 0.0f));
	glm::mat4 handPos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));

	upperArm->setMatrix(upperPos);
	foreArm->setMatrix(forePos);
	hand->setMatrix(handPos);

	// Verify initial world position of hand
	glm::mat4 handWorld = hand->getWorldCoordinateMatrix();
	glm::vec3 handWorldPos = glm::vec3(handWorld[3]);
	assert(vec3Equal(handWorldPos, glm::vec3(0.0f, 10.0f, 0.0f)));

	// Animate upper arm rotation
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	upperArm->move(rotation, 90); // 90 degree rotation over 90 steps

	assert(upperArm->isMoving() == true);

	// Execute animation steps
	for (int i = 0; i < 90; i++)
	{
		upperArm->calculateMove();
	}

	assert(upperArm->isMoving() == false);

	delete armature;

	TEST_PASS();
}

void testMaterialTextureAssignment()
{
	TEST("Material and texture assignment to mesh");

	// Create mesh
	std::vector<glm::vec3> vertices = {
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)};

	std::vector<glm::uvec3> faces = {glm::uvec3(0, 1, 2)};
	std::vector<glm::vec4> normals(3, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
	std::vector<glm::vec2> texCoords(3, glm::vec2(0.0f, 0.0f));

	Eng::Mesh *mesh = new Eng::Mesh("TexturedMesh", glm::mat4(1.0f),
									vertices, faces, normals, texCoords);

	// Create material with properties
	glm::vec4 blue(0.2f, 0.2f, 0.8f, 1.0f);
	Eng::Material *material = new Eng::Material(
		"BlueMaterial",
		glm::vec4(0.0f), // emission
		blue * 0.2f,	 // ambient
		blue * 0.8f,	 // diffuse
		glm::vec4(0.5f), // specular
		64.0f			 // shininess
	);

	// Assign material to mesh
	mesh->setMaterial(material);
	assert(mesh->getMaterial() == material);
	assert(mesh->getMaterial()->getName() == "BlueMaterial");

	// Verify material properties
	Eng::Material *meshMat = mesh->getMaterial();
	assert(meshMat->getDiffuse() == blue * 0.8f);
	assert(floatEqual(meshMat->getShininess(), 64.0f));

	delete mesh;
	delete material;

	TEST_PASS();
}

void testMultipleLights()
{
	TEST("Multiple lights in scene");

	Eng::Node *root = new Eng::Node("Root");

	// Create different light types
	Eng::OmniLight *light1 = new Eng::OmniLight("KeyLight");
	Eng::OmniLight *light2 = new Eng::OmniLight("FillLight");
	Eng::SpotLight *light3 = new Eng::SpotLight("RimLight");
	Eng::InfiniteLight *light4 = new Eng::InfiniteLight("AmbientLight");

	// Position lights
	light1->setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f)));
	light2->setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 5.0f, 5.0f)));
	light3->setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 10.0f)));

	// Set light colors
	light1->setDiffuse(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light2->setDiffuse(glm::vec4(0.5f, 0.5f, 0.8f, 1.0f));
	light3->setDiffuse(glm::vec4(1.0f, 0.9f, 0.7f, 1.0f));
	light4->setDiffuse(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));

	// Add to scene
	root->addChild(light1);
	root->addChild(light2);
	root->addChild(light3);
	root->addChild(light4);

	assert(root->getNumberOfChildren() == 4);

	// Verify light IDs are unique
	int id1 = light1->getLightID();
	int id2 = light2->getLightID();
	int id3 = light3->getLightID();
	int id4 = light4->getLightID();

	assert(id1 != id2);
	assert(id1 != id3);
	assert(id1 != id4);
	assert(id2 != id3);
	assert(id2 != id4);
	assert(id3 != id4);

	delete root;

	TEST_PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(int argc, char *argv[])
{
	std::cout << "========================================" << std::endl;
	std::cout << "   Engine End-to-End Tests" << std::endl;
	std::cout << "   Group 10 - Tower of Hanoi Project" << std::endl;
	std::cout << "========================================" << std::endl;

	// Object tests
	testObjectCreation();

	// Node tests
	testNodeHierarchy();
	testNodeTransformation();
	testNodeWorldCoordinates();
	testNodeMovement();
	testNodeRemoval();

	// Camera tests
	testPerspectiveCamera();
	testOrthoCamera();

	// Material tests
	testMaterial();

	// Mesh tests
	testMeshCreation();

	// Light tests
	testOmniLight();
	testSpotLight();
	testInfiniteLight();

	// List tests
	testListManagement();

	// Complex integration tests
	testComplexSceneGraph();
	testAnimatedHierarchy();
	testMaterialTextureAssignment();
	testMultipleLights();

	// Final report
	std::cout << "\n========================================" << std::endl;
	std::cout << "   TEST RESULTS" << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << "Total tests: " << totalTests << std::endl;
	std::cout << "Passed: " << passedTests << std::endl;
	std::cout << "Failed: " << (totalTests - passedTests) << std::endl;

	if (passedTests == totalTests)
	{
		std::cout << "\n✓ ALL TESTS PASSED!" << std::endl;
		return 0;
	}
	else
	{
		std::cout << "\n✗ SOME TESTS FAILED" << std::endl;
		return 1;
	}
}
