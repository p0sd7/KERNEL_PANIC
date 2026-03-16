#include "ConsoleRenderer.h"
#include <algorithm> 
// Реализация Color
Color::Color(short fg, short bg) : foreground(fg), background(bg) {}

// Реализация ScreenCell
ScreenCell::ScreenCell(wchar_t ch, Color col) : character(ch), color(col) {}

// Реализация ConsoleRenderer
ConsoleRenderer::ConsoleRenderer(int w, int h, bool useDoubleBuffering)
    : width(w), height(h), doubleBuffering(useDoubleBuffering) {

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetupConsole();
    InitializeBuffers();
}

ConsoleRenderer::~ConsoleRenderer() {
    // Возвращаем курсор и сбрасываем цвета при разрушении
    SetCursorVisibility(true);
    ResetConsoleColor();
    SetConsoleCursorPosition(hConsole, { 0, (SHORT)height });
}

void ConsoleRenderer::SetupConsole() {
    // Настройка кодовых страниц для Unicode
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Включение виртуального терминала для ANSI последовательностей
    DWORD mode = 0;
    GetConsoleMode(hConsole, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, mode);

    // Настройка потоков для Unicode
    _setmode(_fileno(stdout), _O_U16TEXT);  

    // Изменение размера буфера консоли
    COORD bufferSize = { (SHORT)width, (SHORT)height };
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // Установка размера окна консоли
    SMALL_RECT windowSize = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    // Скрытие курсора по умолчанию
    SetCursorVisibility(false);
}

void ConsoleRenderer::InitializeBuffers() {
    buffer.resize(height, std::vector<ScreenCell>(width, ScreenCell()));
    if (doubleBuffering) {
        backBuffer.resize(height, std::vector<ScreenCell>(width, ScreenCell()));
    }
}

void ConsoleRenderer::SetPixel(int x, int y, wchar_t ch, Color color) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;

    if (doubleBuffering) {
        backBuffer[y][x] = ScreenCell(ch, color);
    }
    else {
        buffer[y][x] = ScreenCell(ch, color);
        SetConsoleCursorPosition(hConsole, { (SHORT)x, (SHORT)y });
        SetConsoleColor(color);
        std::wcout << ch;
    }
}

void ConsoleRenderer::Clear(Color clearColor) {
    if (doubleBuffering) {
        for (auto& row : backBuffer) {
            for (auto& cell : row) {
                cell = ScreenCell(L' ', clearColor);
            }
        }
    }
    else {
        for (auto& row : buffer) {
            for (auto& cell : row) {
                cell = ScreenCell(L' ', clearColor);
            }
        }
        system("cls");
    }
}

void ConsoleRenderer::Render() {
    if (!doubleBuffering) return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (buffer[y][x].character != backBuffer[y][x].character ||
                buffer[y][x].color.foreground != backBuffer[y][x].color.foreground ||
                buffer[y][x].color.background != backBuffer[y][x].color.background) {

                SetConsoleCursorPosition(hConsole, { (SHORT)x, (SHORT)y });
                SetConsoleColor(backBuffer[y][x].color);
                std::wcout << backBuffer[y][x].character;

                buffer[y][x] = backBuffer[y][x];
            }
        }
    }
}

void ConsoleRenderer::DrawText(int x, int y, const std::wstring& text, Color color) {
    for (size_t i = 0; i < text.length(); i++) {
        // ИСПРАВЛЕНО: явное приведение size_t к int
        SetPixel(x + static_cast<int>(i), y, text[i], color);
    }
}

void ConsoleRenderer::DrawBox(int x1, int y1, int x2, int y2, wchar_t borderChar, Color color) {
    // Верхняя и нижняя границы
    for (int x = x1; x <= x2; x++) {
        SetPixel(x, y1, borderChar, color);
        SetPixel(x, y2, borderChar, color);
    }
    // Боковые границы
    for (int y = y1 + 1; y < y2; y++) {
        SetPixel(x1, y, borderChar, color);
        SetPixel(x2, y, borderChar, color);
    }
}

void ConsoleRenderer::DrawFilledRect(int x1, int y1, int x2, int y2, wchar_t fillChar, Color color) {
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            SetPixel(x, y, fillChar, color);
        }
    }
}

void ConsoleRenderer::DrawLine(int x1, int y1, int x2, int y2, wchar_t lineChar, Color color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        SetPixel(x1, y1, lineChar, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void ConsoleRenderer::DrawLocation(int offsetX, int offsetY, const std::vector<std::wstring>& locationData) {
    for (size_t y = 0; y < locationData.size(); y++) {
        for (size_t x = 0; x < locationData[y].length(); x++) {
            // ИСПРАВЛЕНО: явное приведение size_t к int
            int targetX = offsetX + static_cast<int>(x);
            int targetY = offsetY + static_cast<int>(y);
            if (targetX >= 0 && targetX < width && targetY >= 0 && targetY < height) {
                SetPixel(targetX, targetY, locationData[y][x]);
            }
        }
    }
}

void ConsoleRenderer::DrawLocationWithColors(int offsetX, int offsetY,
    const std::vector<std::wstring>& locationData,
    const std::vector<std::vector<Color>>& colorData) {
    size_t minHeight = min(locationData.size(), colorData.size());

    for (size_t y = 0; y < minHeight; y++) {
        size_t minWidth = min(locationData[y].length(), colorData[y].size());
        for (size_t x = 0; x < minWidth; x++) {
            // ИСПРАВЛЕНО: явное приведение size_t к int
            int targetX = offsetX + static_cast<int>(x);
            int targetY = offsetY + static_cast<int>(y);
            if (targetX >= 0 && targetX < width && targetY >= 0 && targetY < height) {
                SetPixel(targetX, targetY, locationData[y][x], colorData[y][x]);
            }
        }
    }
}

int ConsoleRenderer::GetWidth() const {
    return width;
}

int ConsoleRenderer::GetHeight() const {
    return height;
}

HANDLE ConsoleRenderer::GetConsoleHandle() const {
    return hConsole;
}

void ConsoleRenderer::SetTitle(const std::wstring& title) {
    SetConsoleTitleW(title.c_str());
}

void ConsoleRenderer::SetCursorVisibility(bool visible) {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void ConsoleRenderer::SetConsoleSize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;

    COORD bufferSize = { (SHORT)width, (SHORT)height };
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    SMALL_RECT windowSize = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    InitializeBuffers();
}

int ConsoleRenderer::GetKeyPress() {
    if (_kbhit()) {
        return _getch();
    }
    return 0;
}

int ConsoleRenderer::WaitForKeyPress() {
    return _getch();
}

bool ConsoleRenderer::IsKeyPressed(int keyCode) {
    return (GetAsyncKeyState(keyCode) & 0x8000) != 0;
}

void ConsoleRenderer::SetConsoleColor(Color color) {
    SetConsoleTextAttribute(hConsole, color.foreground | (color.background << 4));
}

void ConsoleRenderer::ResetConsoleColor() {
    SetConsoleTextAttribute(hConsole, Color::WHITE | (Color::BLACK << 4));
}