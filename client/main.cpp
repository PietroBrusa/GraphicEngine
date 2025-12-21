/**
 * @file		main.cpp
 * @brief	Client application (that uses the graphics engine)
 *
 * @author	Achille Peternier (C) SUPSI [achille.peternier@supsi.ch] << change this to your group members
 */

//////////////
// #INCLUDE //
//////////////

// Library header:
#include "engine.h"

// C/C++:
#include <cstdio>
#include <iostream>

// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

/////////////
// GLOBALS //
/////////////

typedef struct dim {
    Eng::Node* node;
    int dim;
} size;

// Cameras:
Eng::Camera* mainCamera;
Eng::Camera* cameras[2];
unsigned short currentCamera{0};

Eng::SpotLight spot;
Eng::Node* root;
Eng::Node* selected_Disk = nullptr;

std::vector<Eng::Node*> sticks;
std::vector<Eng::Node*> disks_a;
std::vector<Eng::Node*> disks_b;
std::vector<Eng::Node*> disks_c;
std::vector<std::vector<Eng::Node*>> m;
bool isWireFrameMode = false;
bool gameWin = false;
int move_selector = 0;
int move = 0;
std::vector<std::vector<std::vector<Eng::Node*>>> memento;
std::vector<size> sizes;

int selector = 0;

///////////////////////////
// TRANSOFRMATIONS TOOLS //
///////////////////////////

static void removeScaling(Eng::Node* node) {
    glm::mat4 localMatrix = node->getMatrix();

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(localMatrix, scale, rotation, translation, skew, perspective);

    glm::vec3 inverseScale = glm::vec3(1.0) / scale;
    glm::mat4 inverseScaleMatrix = glm::scale(glm::mat4(1.0), inverseScale);
    glm::mat4 normalizedlocalMatrix = inverseScaleMatrix * localMatrix;
    node->setMatrix(normalizedlocalMatrix);
}

static void removeTranslation(Eng::Node* node) {
    glm::mat4 localMatrix = node->getMatrix();

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(localMatrix, scale, rotation, translation, skew, perspective);

    glm::vec3 inverseTraslation = -translation;
    glm::mat4 inverseTranslationMatrix = glm::translate(glm::mat4(1.0), inverseTraslation);
    glm::mat4 normalizedlocalMatrix = inverseTranslationMatrix * localMatrix;
    node->setMatrix(normalizedlocalMatrix);
}

static void removeRotation(Eng::Node* node) {
    glm::mat4 localMatrix = node->getMatrix();

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(localMatrix, scale, rotation, translation, skew, perspective);

    glm::mat4 rotationMatrix = glm::mat4_cast(rotation);

    glm::mat4 inverseRotation = glm::inverse(rotationMatrix);

    glm::mat4 normalizedlocalMatrix = localMatrix * inverseRotation;
    node->setMatrix(normalizedlocalMatrix);
}

///////////////////
// GLOWING TOOLS //
///////////////////

static Eng::Material* getGlowingMaterial() {
    static Eng::Material* glowingMaterial = new Eng::Material(
        "Glowing",
        glm::vec4(1.0f, 0.794f, 0.0f, 1.0f),
        glm::vec4(0.1f, 0.08f, 0.0f, 1.0f),
        glm::vec4(0.4f, 0.32f, 0.0f, 1.0f));
    return glowingMaterial;
}

static void glowingDiskAt(unsigned int index) {
    static Eng::Mesh* lastGlowedDisk = nullptr;
    static Eng::Material* diskMaterial = nullptr;

    if (selected_Disk)
        return;

    if (lastGlowedDisk)  // Reset glowing disk
    {
        lastGlowedDisk->setMaterial(diskMaterial);
        lastGlowedDisk = nullptr;
        diskMaterial = nullptr;
    }

    if (index >= m.size())  // negative excluded
        return;

    std::vector<Eng::Node*> stickDisks = m[index];

    if (stickDisks.empty())
        return;

    lastGlowedDisk = dynamic_cast<Eng::Mesh*>(stickDisks.back());

    if (lastGlowedDisk == nullptr)
        return;

    diskMaterial = lastGlowedDisk->getMaterial();

    lastGlowedDisk->setMaterial(getGlowingMaterial());
}

///////////////////
// MEMENTO TOOLS //
///////////////////

bool checkselectorVal(int selector) {
    return selector >= 0 && selector <= 2;
}

bool checkMementoMove(int move) {
    return move >= 0 && move < memento.size();
}

static void addMove() {
    memento.push_back(m);
    move++;
    move_selector++;
}

static void applyMemento(std::vector<std::vector<Eng::Node*>> config) {
    if (selected_Disk) {
        selected_Disk->resetMove();
        removeRotation(selected_Disk);
        selected_Disk = nullptr;
    }

    for (size_t stick = 0; stick < config.size(); stick++) {
        glm::mat4 h = glm::mat4(1.0);

        for (size_t disk = 0; disk < config[stick].size(); disk++) {
            h = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, disk * 0.25f + 0.25f / 2, 0.0f));
            Eng::Node* n = config[stick][disk];

            n->resetMove();
            removeRotation(n);

            removeTranslation(n);
            n->setMatrix(h * n->getMatrix());
            sticks[stick]->addChild(n);
        }
    }
    m = config;

    // Reset glowing disk
    glowingDiskAt(selector);
}

static void resetGame() {
    move = 0;
    applyMemento(memento[move]);
    memento.clear();
}

///////////////////
// WINNING TOOLS //
///////////////////

static bool checkForWin() {
    if (!m[0].empty())
        return false;  // Some disk on first stick

    if (!m[1].empty() && !m[2].empty())
        return false;  // Disks are on the others sticks

    return true;  // Disks are all in one stick
}

static void resetAndStartNewTunr() {
    gameWin = false;
    resetGame();
}

static void drawWinScreen(Eng::GUIObjects* gui) {
    int screenWidth = gui->getWidth();
    int screenHeight = gui->getHeight();

    if (screenWidth == 0 || screenHeight == 0) {
        return;
    }

    glm::vec4 overlayColor(0.0f, 0.0f, 0.0f, 0.7f);  // Black, 70% opaque
    glm::vec4 textColor(1.0f, 1.0f, 1.0f, 1.0f);     // Solid White

    // --- Text Metrics Setup ---

    const unsigned char* line1 = (const unsigned char*)"YOU WIN";
    const unsigned char* line2 = (const unsigned char*)"Press any key to restart";

    int charWidth = 8;
    int charHeight = 13;

    float width1 = (float)strlen((const char*)line1) * charWidth;
    float width2 = (float)strlen((const char*)"Press any key to restart") * charWidth;

    float contentWidth = std::max(width1, width2);

    float padding = 20.0f;
    float lineSpacing = (float)charHeight * 1.5f;

    // --- Bounding Box Calculation ---

    float boxWidth = contentWidth + (padding * 2.0f);
    float boxHeight = (float)charHeight + lineSpacing + (float)charHeight + (padding * 2.0f);

    // --- Centering Calculations ---

    float boxX_Start = ((float)screenWidth / 2.0f) - (boxWidth / 2.0f);
    float boxY_Start = ((float)screenHeight / 2.0f) + (boxHeight / 2.0f);

    // --- Drawing the Background Panel ---

    gui->drawRect(
        glm::vec2(boxX_Start, boxY_Start),
        boxWidth,
        boxHeight,
        overlayColor);

    // --- Drawing the Text (Centering relative to the box) ---

    float centerY = (float)screenHeight / 2.0f;

    float centerX1 = boxX_Start + padding + ((contentWidth - width1) / 2.0f);
    float centerX2 = boxX_Start + padding + ((contentWidth - width2) / 2.0f);

    float line1_Y = centerY + (lineSpacing / 2.0f);
    float line2_Y = centerY - (lineSpacing / 2.0f);

    gui->drawText(glm::vec2(centerX1, line1_Y), line1, textColor);
    gui->drawText(glm::vec2(centerX2, line2_Y), line2, textColor);
}

/////////////////
// DISKS TOOLS //
/////////////////

static int getDiskDim(Eng::Node* node) {
    for (size_t i = 0; i < sizes.size(); i++) {
        if (sizes[i].node == node) {
            return sizes[i].dim;
        }
    }
    return 99;
}

static void moveDiskHorizzontally(Eng::Node* spot, bool direction) {
    glm::mat4 previousMatrix = spot->getWorldCoordinateMatrix();

    selector = selector + (direction ? +1 : -1);
    sticks[selector]
        ->addChild(spot);

    glm::mat4 delta = glm::inverse(previousMatrix) * spot->getWorldCoordinateMatrix();
    spot->setMatrix(spot->getMatrix() * glm::inverse(delta));

    spot->move(glm::translate(glm::mat4(1.0f), glm::vec3((direction ? 1 : -1) * 0.1f, 0.0f, 0.0f)), 25);
}

static void putSelectedDiskDown() {
    selected_Disk->resetMove();
    removeRotation(selected_Disk);
    glm::mat4 previousMatrix = selected_Disk->getWorldCoordinateMatrix();

    int nDisks = m[selector].size();
    m[selector].push_back(selected_Disk);

    sticks[selector]->addChild(selected_Disk);
    removeTranslation(selected_Disk);

    glm::mat4 delta = glm::inverse(previousMatrix) * selected_Disk->getWorldCoordinateMatrix();
    selected_Disk->setMatrix(selected_Disk->getMatrix() * glm::inverse(delta));

    selected_Disk->move(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.05f, 0.0f)), 15);

    // go down the number of diks that aren't there
    selected_Disk->move(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.05f, 0.0f)), 5 * (7 - (nDisks + 1)));
    selected_Disk = nullptr;

    if (checkForWin()) {
        std::cout << "You win!" << std::endl;
        gameWin = true;
    }

    // add move
    addMove();
}

static void selectDisk(Eng::Node* spot) {
    int nDisks = m[selector].size();
    int last = nDisks - 1;
    selected_Disk = m[selector][last];
    m[selector].pop_back();

    glm::mat4 previousMatrix = selected_Disk->getWorldCoordinateMatrix();

    spot->addChild(selected_Disk);
    removeTranslation(selected_Disk);

    glm::mat4 delta = glm::inverse(previousMatrix) * selected_Disk->getWorldCoordinateMatrix();
    selected_Disk->setMatrix(selected_Disk->getMatrix() * glm::inverse(delta));

    selected_Disk->setMatrix(selected_Disk->getMatrix() * glm::rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f)));

    selected_Disk->move(glm::rotate(glm::mat4(1.0f), glm::radians(4.0f), glm::vec3(0.0f, 1.0f, 0.0f)), -1);
    selected_Disk->move(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.05f, 0.0f)), 15);

    // go up the number of diks that aren't there
    selected_Disk->move(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.05f, 0.0f)), 5 * (7 - nDisks));
}

///////////////
// CALLBACKS //
///////////////

static void renderingLoop(Eng::Node* root) {
}

static void onTextDrawCallback(Eng::GUIObjects gui) {
    char text[64];
    int fps = Eng::Base::getInstance().getCurrentFPS();

#if defined(WIN32) || defined(_WIN32)
    sprintf_s(text, sizeof(text), "FPS: %d | Moves: %d", fps, move);
#else
    snprintf(text, sizeof(text), "FPS: %d | Moves: %d", fps, move);
#endif

    gui.drawText(glm::vec2(1.0f, 2.0f), (unsigned char*)text, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    if (gameWin) {
        drawWinScreen(&gui);
    }
}

static void onResizeCallback(int width, int height) {
}

static void onSpecialPressedCallback(int key, int mouseX, int mouseY) {
    if (gameWin) {
        // Reset game
        resetAndStartNewTunr();
        return;
    }

    Eng::Node* spot = root->getChild("Spot");

    if (spot) {
        glm::mat4 up = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, +0.1f, 0.0f));
        glm::mat4 down = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -100.0f, 0.0f));

        switch (key) {
            case ENGINE_KEY_UP:

                while (spot->isMoving()) {
                    spot->calculateMove();
                }
                if (selected_Disk) {
                    while (selected_Disk->isMoving()) {
                        selected_Disk->calculateMove();
                    }
                }

                if (!selected_Disk && m[selector].size() > 0) {
                    selectDisk(spot);
                }
                break;

            case ENGINE_KEY_DOWN:
                while (spot->isMoving()) {
                    spot->calculateMove();
                }
                if (selected_Disk) {
                    while (selected_Disk->isMoving()) {
                        selected_Disk->calculateMove();
                    }
                }
                if (selected_Disk && (m[selector].size() == 0 || getDiskDim(selected_Disk) < getDiskDim(m[selector][m[selector].size() - 1]))) {
                    putSelectedDiskDown();
                }
                break;

            case ENGINE_KEY_LEFT:
                if (checkselectorVal(selector - 1)) {
                    while (spot->isMoving()) {
                        spot->calculateMove();
                    }
                    while (selected_Disk && selected_Disk->isMoving()) {
                        selected_Disk->calculateMove();
                    }

                    moveDiskHorizzontally(spot, false);
                }

                glowingDiskAt(selector);
                break;

            case ENGINE_KEY_RIGHT:
                if (checkselectorVal(selector + 1)) {
                    while (spot->isMoving()) {
                        spot->calculateMove();
                    }
                    while (selected_Disk && selected_Disk->isMoving()) {
                        selected_Disk->calculateMove();
                    }

                    moveDiskHorizzontally(spot, true);
                }

                glowingDiskAt(selector);
                break;

            case ENGINE_KEY_SHIFT_L:
                mainCamera->setMatrix(glm::translate(mainCamera->getMatrix(), glm::vec3(0.0f, 1.0f, 0.0f)));
                break;

            case ENGINE_KEY_CTRL_L:
                mainCamera->setMatrix(glm::translate(mainCamera->getMatrix(), glm::vec3(0.0f, -1.0f, 0.0f)));
                break;
        }
    } else {
        std::cout << "Wireframe mode\n"
                  << std::endl;
    }
}

static void onKeyboardPressedCallback(unsigned char key, int mouseX, int mouseY) {
    if (gameWin) {
        // Reset game
        resetAndStartNewTunr();
        return;
    }

    if (key == 'k') {
        isWireFrameMode = !isWireFrameMode;
        Eng::Base::getInstance().changeWireFrame(isWireFrameMode);
    }

    if (key == 'l') {
        resetGame();
    }

    if (key == 'a') {
        mainCamera->setMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * mainCamera->getMatrix());
    } else if (key == 'd') {
        mainCamera->setMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * mainCamera->getMatrix());
    } else if (key == 'w') {
        mainCamera->setMatrix(glm::translate(mainCamera->getMatrix(), glm::vec3(0.0f, 0.0f, -1.0f)));
    } else if (key == 's') {
        mainCamera->setMatrix(glm::translate(mainCamera->getMatrix(), glm::vec3(0.0f, 0.0f, 1.0f)));
    } else if (key == 'm') {
        mainCamera->move(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f)), 10);
    } else if (key == 'r') {
        if (mainCamera->isMoving()) {
            mainCamera->stopMove();
            return;
        }

        mainCamera->move(glm::rotate(glm::mat4(1.0f), glm::radians(1.0f / 4), glm::vec3(0.0f, 1.0f, 0.0f)), 360 * 4);
    } else if (key == 'e') {
        mainCamera->setMatrix(mainCamera->getMatrix() * glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    } else if (key == 'q') {
        mainCamera->setMatrix(mainCamera->getMatrix() * glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
    } else if (key == 'f') {
        mainCamera = cameras[currentCamera++ % 2];
        std::cout << "Current camera: " << mainCamera->getName() << std::endl;
        Eng::Base::getInstance().setActiveCamera(mainCamera);
    } else if (key == 'i') {
        if (selected_Disk) {
            std::cout << "Cannot undo while holding a disk!" << std::endl;
            return;
        }
        if (checkMementoMove(move - 1)) {
            move--;
            applyMemento(memento[move]);
        }
    } else if (key == 'o') {
        if (selected_Disk) {
            std::cout << "Cannot redo while holding a disk!" << std::endl;
            return;
        }
        if (checkMementoMove(move + 1)) {
            move++;
            applyMemento(memento[move]);
        }
    }
}

static void onMouseCallback(int buttonId, int buttonState, int mouseX, int mouseY) {
    printf("memento: %d\n", memento.size());
}

static void onIdleCallback() {
}

static void onCloseCallback() {
}

//////////
// MAIN //
//////////

/**
 * Application entry point.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char* argv[]) {
    // Credits:
    std::cout << "Hanoi Tower Client application, Group 10 (C) SUPSI" << std::endl;
    std::cout << std::endl;

    glm::mat4 cameraPPosition = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 8.0f, 15.0f));
    Eng::PerspectiveCamera* perspectiveCamera = new Eng::PerspectiveCamera("mainPerpectiveCamera", cameraPPosition);
    perspectiveCamera->setCameraParams(45.0f, RATIO_16_9, 1.0f, 5000.0f);

    glm::mat4 cameraOPosition = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    cameraOPosition = glm::translate(cameraOPosition, glm::vec3(0.0f, 0.0f, 15.0f));
    Eng::OrthoCamera* orthoCamera = new Eng::OrthoCamera("mainOrthoCamera", cameraOPosition);
    orthoCamera->setCameraParams(30, 20, 1.0f, 5000.0f);

    // Add camera
    cameras[0] = perspectiveCamera;
    cameras[1] = orthoCamera;

    mainCamera = cameras[currentCamera++];

    glm::mat4 ry = glm::rotate(glm::mat4(1.0f), glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mainCamera->setViewMatrix(ry * mainCamera->getMatrix());

    // Init engine:.
    Eng::Base& eng = Eng::Base::getInstance();
    eng.setActiveCamera(mainCamera);
    eng.init(&argc, argv, "CG Project");
    root = eng.loadScene("./res/hanoitower.ovo");

    // Initialize scene [START]

    for (size_t i = 0; i < root->getChildren().size(); i++) {
        Eng::Node* child = root->getChildren()[i];
    }

    int selector = 0;
    sticks.push_back(root->getChild("Stick1"));
    sticks.push_back(root->getChild("Stick2"));
    sticks.push_back(root->getChild("Stick3"));

    disks_a.push_back(root->getChild("Black_cilinder"));
    disks_a.push_back(root->getChild("Purple_cilinder"));
    disks_a.push_back(root->getChild("Blue_cilinder"));
    disks_a.push_back(root->getChild("Green_cilinder"));
    disks_a.push_back(root->getChild("Yellow_cilinder"));
    disks_a.push_back(root->getChild("Orange_cilinder"));
    disks_a.push_back(root->getChild("Red_cilinder"));

    for (size_t i = 0; i < disks_a.size(); i++) {
        size s;

        s.dim = disks_a.size() - i;
        s.node = disks_a[i];
        sizes.push_back(s);
    }

    m.push_back(disks_a);
    m.push_back(disks_b);
    m.push_back(disks_c);
    memento.push_back(m);
    // prova

    glowingDiskAt(selector);

    // Initialize scene [END]

    glm::mat4 lightNodePosition = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 500.0f, 0.0f));
    glm::mat4 lightStickTrasl = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, 0.0f, 0.0f));

    root->addChild(perspectiveCamera);
    root->addChild(orthoCamera);

    // Set callbacks
    eng.setOnResizeCallback(onResizeCallback);
    eng.setOnSpecialPressedCallback(onSpecialPressedCallback);
    eng.setOnKeyboardPressedCallback(onKeyboardPressedCallback);
    eng.setOnMouseCallback(onMouseCallback);
    eng.setOnIdleCallback(onIdleCallback);
    eng.setOnCloseCallback(onCloseCallback);
    eng.setOnTextDrawCallback(onTextDrawCallback);

    eng.start(renderingLoop);

    // Release engine:
    eng.free();

    // Free resources:
    delete perspectiveCamera;
    delete orthoCamera;

    Eng::Material* materialCleanup = getGlowingMaterial();
    if (materialCleanup) {
        delete materialCleanup;
        materialCleanup = nullptr;
    }

    // Done:
    std::cout << "\n[application terminated]" << std::endl;
    return 0;
}
