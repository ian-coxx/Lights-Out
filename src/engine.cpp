#include "engine.h"
#include <sstream>
#include <cstdlib>
#include <ctime>

enum state {start, play, over};
state screen;

// Colors
color yellow, grey, red, hoverFill, pressFill;

// Time-related variables
bool gameStarted = false;
int startTime = 0;
int endTime = 0;

int clicks = 0;

Engine::Engine() : keys() {
    red = {1, 0, 0, 1};
    yellow = {1, 1, 0, 1};
    grey = {.5, .5, .5, 1};
    hoverFill.vec = red.vec + vec4{0.5, 0.5, 0.5, 0};
    pressFill.vec = red.vec - vec4{0.5, 0.5, 0.5, 0};

    this->initWindow();
    this->initShaders();
    this->initShapes();
    //this->randomizeGrid();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    // red spawn button centered in the top left corner
    user = make_unique<Rect>(shapeShader, vec2{width/2,height/2.2}, vec2{70, 35}, red);
    int x;
    int y = 0;

    for (int i = 0; i < 5; i++) {
        x = 0;
        for (int j = 0; j < 5; j++) {
            vec2 pos = {150 + x, 50 + y};
            vec2 size = {90, 90};
            color color = yellow;
            buttons.push_back(make_unique<Rect>(shapeShader, pos, size, color));
            x += 125;
        }
        y += 125;
    }
    border = make_unique<Rect>(shapeShader, vec2{-1000,-1000}, vec2{100, 100}, red);
}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // If we're in the start screen and the user presses s, change screen to play
    if (screen == start && keys[GLFW_KEY_S]) {
        screen = play;
        gameStarted = true;
        startTime = glfwGetTime();
    }

    // Mouse position is inverted because the origin of the window is in the top left corner
    MouseY = height - MouseY; // Invert y-axis of mouse position
    bool buttonOverlapsMouse = user->isOverlapping(vec2(MouseX, MouseY));
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    //When in play screen, if the user hovers or clicks on the button then change the button's color
    if (screen == play) {
        for (int i = 0; i < buttons.size(); i++) {
            if (buttons[i]->isOverlapping(vec2(MouseX, MouseY))) {
                border->setPos(vec2{buttons[i]->getPosX(), buttons[i]->getPosY()});
                if (mousePressedLastFrame && !mousePressed) {
                    clicks++;
                    if (buttons[i]->getRight() == 695) {
                        surrounding.push_back(i - 1);
                        surrounding.push_back(i + 5);
                        surrounding.push_back(i - 5);
                    } else if (buttons[i]->getRight() == 195) {
                        surrounding.push_back(i + 1);
                        surrounding.push_back(i + 5);
                        surrounding.push_back(i - 5);
                    } else {
                        surrounding.push_back(i + 1);
                        surrounding.push_back(i - 1);
                        surrounding.push_back(i + 5);
                        surrounding.push_back(i - 5);
                    }
                    for (int j = 0; j < surrounding.size(); j++) {
                        int index = surrounding[j];
                        if (index >= 0 && index < 25) {
                            if (buttons[index]->getBlue() == 0.0) {
                                buttons[index]->setColor(grey);
                            } else {
                                buttons[index]->setColor(yellow);
                            }
                        }
                    }
                    if (buttons[i]->getBlue() == 0.0) {

                        buttons[i]->setColor(grey);
                    } else {
                        buttons[i]->setColor(yellow);
                    }
                    surrounding.clear();
                }
            }
        }
        int count = 0;
        for (int i = 0; i < buttons.size(); i++) {
            if (buttons[i]->getBlue() == .5) {
                count++;
                if (count == 25) {
                    screen = over;
                }
            }
        }
    }
    // Save mousePressed for next frame
    mousePressedLastFrame = mousePressed;

}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (gameStarted && screen == over && endTime == 0) {
        endTime = currentFrame;
    }
}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to use for all shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            string message = "Press s to start!";
            // (12 * message.length()) is the offset to center text.
            // 12 pixels is the width of each character scaled by 1.
            // Game instructions
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), 100, 1, vec3{1, 0, 0});
            string instMessage = "How to Play:";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 500, 1, vec3{1, 1, 1});
            instMessage = "You have a 5x5 grid of lights.";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 450, 1, vec3{1, 1, 1});
            instMessage = "When you click on a light, it";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 400, 1, vec3{1, 1, 1});
            instMessage = "And all adjacent lights are set";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 350, 1, vec3{1, 1, 1});
            instMessage = "On if they're off and vice versa";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 300, 1, vec3{1, 1, 1});
            instMessage = "Your goal is to turn all of the";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 250, 1, vec3{1, 1, 1});
            instMessage = "'Lights out!'";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 200, 1, vec3{1, 1, 1});
            instMessage = "Good luck!";
            this->fontRenderer->renderText(instMessage, width/2 - (12 * instMessage.length()), 150, 1, vec3{1, 1, 1});
            break;
        }
        case play: {
            // draw user first so it is hidden behind lights
            user->setUniforms();
            user->draw();

            border->setUniforms();
            border->draw();

            // display all lights
            for (const unique_ptr<Shape> &b : buttons) {
                b->setUniforms();
                b->draw();
            }

            // displays the number of clicks throughout the game
            stringstream ss;
            ss << clicks;
            string message = ss.str();
            ss << message;
            string clickTracker = "# of Clicks: " + message;
            this->fontRenderer->renderText(clickTracker, width/2 - (12 * clickTracker.length()), 540, 1, vec3{1, 1, 1});
            break;
        }
        case over: {
            // display all lights again
            for (const unique_ptr<Shape> &b : buttons) {
                b->setUniforms();
                b->draw();
            }

            int totalTime = endTime - startTime;
            stringstream ss;
            ss << totalTime;
            string message = "You win!";
            // Displays the message on the screen
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), 540, 1, vec3{1, 1, 0});
            string timeText = "It took you...";
            this->fontRenderer->renderText(timeText, width/2 - (12 * timeText.length()), 460, 1, vec3{1, 1, 1});
            string timeResult = " ";
            ss >> timeResult;
            this->fontRenderer->renderText(timeResult, width/2 - (12 * timeResult.length()), 407, 1, vec3{1, 1, 1});
            string timeQ = "seconds!";
            this->fontRenderer->renderText(timeQ, width/2 - (12 * timeQ.length()), 355, 1, vec3{1, 1, 1});
            stringstream ss1;
            ss1 << clicks;
            string clix = "";
            ss1 >> clix;
            string clicksResult = "# of Clicks: " + clix;
            this->fontRenderer->renderText(clicksResult, width/2 - (12 * clicksResult.length()), 300, 1, vec3{1, 1, 1});
            break;
        }
    }

    glfwSwapBuffers(window);
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}