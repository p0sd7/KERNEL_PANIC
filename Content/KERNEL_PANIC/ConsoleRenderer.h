#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include <conio.h>  // ДОБАВЛЕНО: для _kbhit() и _getch()
#include <fcntl.h>
#include <io.h>

// Структура для представления цвета
struct Color {
    short foreground;
    short background;

    Color(short fg = 7, short bg = 0);

    // Стандартные цвета Windows
    static const short BLACK = 0;
    static const short BLUE = 1;
    static const short GREEN = 2;
    static const short CYAN = 3;
    static const short RED = 4;
    static const short MAGENTA = 5;
    static const short YELLOW = 6;
    static const short WHITE = 7;
    static const short GRAY = 8;
    static const short BRIGHT_BLUE = 9;
    static const short BRIGHT_GREEN = 10;
    static const short BRIGHT_CYAN = 11;
    static const short BRIGHT_RED = 12;
    static const short BRIGHT_MAGENTA = 13;
    static const short BRIGHT_YELLOW = 14;
    static const short BRIGHT_WHITE = 15;
};

// Структура для представления ячейки на экране
struct ScreenCell {
    wchar_t character;
    Color color;

    ScreenCell(wchar_t ch = L' ', Color col = Color());
};

// Основной класс рендерера
class ConsoleRenderer {
private:
    int width, height;
    HANDLE hConsole;
    std::vector<std::vector<ScreenCell>> buffer;
    std::vector<std::vector<ScreenCell>> backBuffer;
    bool doubleBuffering;

    // Приватные вспомогательные методы
    void SetupConsole();
    void InitializeBuffers();

public:
    // Конструктор и деструктор
    ConsoleRenderer(int w = 80, int h = 25, bool useDoubleBuffering = true);
    ~ConsoleRenderer();

    // Запрет копирования
    ConsoleRenderer(const ConsoleRenderer&) = delete;
    ConsoleRenderer& operator=(const ConsoleRenderer&) = delete;

    // Базовые методы рисования
    void SetPixel(int x, int y, wchar_t ch, Color color = Color());
    void Clear(Color clearColor = Color());
    void Render();

    // Методы для отрисовки примитивов
    void DrawText(int x, int y, const std::wstring& text, Color color = Color());
    void DrawBox(int x1, int y1, int x2, int y2, wchar_t borderChar = L'#', Color color = Color());
    void DrawFilledRect(int x1, int y1, int x2, int y2, wchar_t fillChar = L' ', Color color = Color());
    void DrawLine(int x1, int y1, int x2, int y2, wchar_t lineChar = L'*', Color color = Color());

    // Методы для отрисовки локаций (принимают данные локации извне)
    void DrawLocation(int offsetX, int offsetY, const std::vector<std::wstring>& locationData);
    void DrawLocationWithColors(int offsetX, int offsetY,
        const std::vector<std::wstring>& locationData,
        const std::vector<std::vector<Color>>& colorData);

    // Геттеры
    int GetWidth() const;
    int GetHeight() const;
    HANDLE GetConsoleHandle() const;

    // Управление консолью
    void SetTitle(const std::wstring& title);
    void SetCursorVisibility(bool visible);
    void SetConsoleSize(int newWidth, int newHeight);

    // Ввод
    int GetKeyPress();              // Неблокирующий ввод
    int WaitForKeyPress();          // Блокирующий ввод
    bool IsKeyPressed(int keyCode); // Проверка конкретной клавиши

    // Работа с цветами
    void SetConsoleColor(Color color);
    void ResetConsoleColor();
};