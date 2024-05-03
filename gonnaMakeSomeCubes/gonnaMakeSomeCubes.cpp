#include <iostream>
#include <Windows.h>
#include "Screen.h"

struct pos {
    int x;
    int y;
};

//pos mapGridToScreen(int x, int y) {
//    return 
//}

pos mapScreenToGrid(int x, int y) {
    return { x / 2, y };
}

void gridInput(Screen& screen, int gridX, int gridY) {
    screen.input('[', gridX*2, gridY);
    screen.input(']', gridX*2 + 1, gridY);
}

void line(Screen& screen, int x1, int y1, int x2, int y2) {
    char chr = '*';

    float gradient = (float)(y2 - y1) / (x2 - x1);

    int lengthX = abs(x2 - x1);
    int lengthY = abs(y2 - y1);

    pos leftPos;
    if (x1 < x2) {
        leftPos = { x1, y1 };
    }
    else {
        leftPos = { x2, y2 };
    }

    pos topPos;
    if (y1 < y2) {
        topPos = { x1, y1 };
    }
    else {
        topPos = { x2, y2 };
    }

    int minX;
    if (x1 < x2) {
        minX = x1;
    }
    else {
        minX = x2;
    }

    int minY;
    if (y1 < y2) {
        minY = y1;
    }
    else {
        minY = y2;
    }
    
    if (lengthX == 0) {
        for (int i = 0; i < lengthY + 1; i++) {
            //screen.input(chr, x1, minY + i);
            gridInput(screen, x1, minY + i);
        }
        return;
    }

    if (lengthY == 0) {
        for (int i = 0; i < lengthX; i++) {
            //screen.input(chr, minX + i, y1);
            gridInput(screen, minX + i, y1);
        }
        return;
    }

    if (abs(gradient) < 1) {
        for (int i = 0; i < lengthX + 1; i++) {
            //screen.input(chr, leftPos.x + i, leftPos.y + round(i * gradient));
            gridInput(screen, leftPos.x + i, leftPos.y + round(i * gradient));
        }
        return;
    }

    for (int i = 0; i < lengthY + 1; i++) {
        //screen.input(chr, topPos.x + round(i * (1 / gradient)), topPos.y + i);
        gridInput(screen, topPos.x + round(i * (1 / gradient)), topPos.y + i);
    }
}

struct vector3 {
    float x;
    float y;
    float z;
};

class Cuboid {
public:
    vector3 nodes[8];
    pos edges[12] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

public:
    Cuboid() {
        for (int i = 0; i < 8; i++) {
            nodes[i] = { 0, 0, 0 };
        }
    }

    Cuboid(float x, float y, float z, float w, float h, float d) {
        vector3 tempNodes[] = {
            {x, y, z},
            {x+w, y, z},
            {x+w, y, z+d},
            {x, y, z+d},
            {x, y+h, z},
            {x+w, y+h, z},
            {x+w, y+h, z+d},
            {x, y+h, z+d}
        };

        for (int i = 0; i < 8; i++) {
            nodes[i] = tempNodes[i];
        }
    }

    Cuboid(const Cuboid& cuboid) {
        for (int i = 0; i < 8; i++) {
            nodes[i] = cuboid.nodes[i];
        }
    }
};

void rotateZ3D(float theta, vector3 nodes[8]) {
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

    for (int i = 0; i < 8; i++) {
        float x = nodes[i].x;
        float y = nodes[i].y;

        nodes[i].x = x * cosTheta - y * sinTheta;
        nodes[i].y = y * cosTheta + x * sinTheta;
    }
}

void rotateX3D(float theta, vector3 nodes[8]) {
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

    for (int i = 0; i < 8; i++) {
        float y = nodes[i].y;
        float z = nodes[i].z;

        nodes[i].y = y * cosTheta + z * sinTheta;
        nodes[i].z = z * cosTheta - y * sinTheta;
    }
}

void rotateY3D(float theta, vector3 nodes[8]) {
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

    for (int i = 0; i < 8; i++) {
        float x = nodes[i].x;
        float z = nodes[i].z;

        nodes[i].x = x * cosTheta + z * sinTheta;
        nodes[i].z = z * cosTheta - x * sinTheta;
    }
}

int main() {
    SetConsoleOutputCP(437);

    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);

    DWORD consoleMode;
    GetConsoleMode(hout, &consoleMode);
    SetConsoleMode(hout, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleMode(hin, ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);

    SHORT width = 200;
    SHORT height = 50;

    SMALL_RECT dim = { 0, 0, width - 1, height - 1 };

    SetConsoleScreenBufferSize(hout, { width, height });

    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);

    GetCurrentConsoleFontEx(hout, false, &cfi);

    cfi.dwFontSize.Y = 18;
    cfi.dwFontSize.X = 10;

    SetCurrentConsoleFontEx(hout, false, &cfi);

    //pretty sure it only needs one redo, so no for loop check
    if (!SetConsoleWindowInfo(hout, true, &dim)) {
        cfi.dwFontSize.Y = 16;
        cfi.dwFontSize.X = 9;

        SetCurrentConsoleFontEx(hout, false, &cfi);

        SetConsoleWindowInfo(hout, true, &dim);
    }

    INPUT_RECORD inputRecord;
    DWORD events;

    Screen screen = Screen(width, height);

    //game ticks per second
    int tickrate = 50;

    int mouseX = 0;
    int mouseY = 0;

    Cuboid cube = Cuboid(-10, -10, -10, 20, 20, 20);
    
    pos offset = { width / 4, height / 2 };

    pos mouseGridPos = { 0, 0 };

    while (true) {
        GetNumberOfConsoleInputEvents(hin, &events);

        if (events > 0) {
            ReadConsoleInput(hin, &inputRecord, 1, &events);
            switch (inputRecord.EventType) {
            case KEY_EVENT:
                switch (inputRecord.Event.KeyEvent.wVirtualKeyCode) {
                case VK_SPACE:
                    if (inputRecord.Event.KeyEvent.bKeyDown) {
                        //spacebar
                    }
                    break;
                case VK_UP:
                    if (inputRecord.Event.KeyEvent.bKeyDown) {
                        rotateX3D(0.1, cube.nodes);
                    }
                    break;
                case VK_RIGHT:
                    if (inputRecord.Event.KeyEvent.bKeyDown) {
                        rotateY3D(-0.1, cube.nodes);
                    }
                    break;
                case VK_DOWN:
                    if (inputRecord.Event.KeyEvent.bKeyDown) {
                        rotateX3D(-0.1, cube.nodes);
                    }
                    break;
                case VK_LEFT:
                    if (inputRecord.Event.KeyEvent.bKeyDown) {
                        rotateY3D(0.1, cube.nodes);
                    }
                    break;
                }
                break;

            case MOUSE_EVENT:
                mouseX = inputRecord.Event.MouseEvent.dwMousePosition.X;
                mouseY = inputRecord.Event.MouseEvent.dwMousePosition.Y;

                mouseGridPos = mapScreenToGrid(mouseX, mouseY);

                if (inputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                    //mouseClick
                }
                break;
            }
        }

        //rotateZ3D(0.1, cube.nodes);
        //rotateY3D(0.2, cube.nodes);

        screen.reset();

        screen.text("Mouse position:", 4, 2);
        screen.text("screen: x:" + std::to_string(mouseX) + ", y:" + std::to_string(mouseY), 6, 3);
        screen.text("grid: x:" + std::to_string(mouseGridPos.x) + ", y:" + std::to_string(mouseGridPos.y), 6, 4);
        line(screen, 75, 25, mouseGridPos.x, mouseGridPos.y);
        
        for (int i = 0; i < 12; i++) {
            vector3 node1 = cube.nodes[cube.edges[i].x];
            vector3 node2 = cube.nodes[cube.edges[i].y];

            line(screen, node1.x + offset.x, node1.y + offset.y, node2.x + offset.x, node2.y + offset.y);
        }

        //mouse cursor
        gridInput(screen, mouseGridPos.x, mouseGridPos.y);

        screen.print();

        Sleep(1000 / tickrate);
    }
}
