#include "ConsoleRenderer.h"
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <map> // ДОБАВЛЕНО

// Класс для управления локациями (отдельно от рендерера)
class LocationManager {
private:
    std::map<std::string, std::vector<std::wstring>> locations;

public:
    LocationManager() {
        CreateLocations();
    }

    void CreateLocations() {
        // Лес
        locations["forest"] = {
            L"╔════════════════════════════════════╗",
            L"║    🌲 ЛЕС 🌲                       ║",
            L"║    🌳    🌳    🌲    🌳            ║",
            L"║       🌲    🌳    🌲                ║",
            L"║    🌲    🌳         🌲              ║",
            L"║         🌳    🌲    🌳    🌲        ║",
            L"║    🌲         🌳                    ║",
            L"║       🏠 ДОМИК 🏠                   ║",
            L"║    🌳    🌲    🌳    🌲             ║",
            L"╚════════════════════════════════════╝"
        };

        // Подземелье
        locations["dungeon"] = {
            L"╔════════════════════════════════════╗",
            L"║    🏰 ПОДЗЕМЕЛЬЕ 🏰                ║",
            L"║    [====]    [####]    [====]      ║",
            L"║      ||        ||        ||        ║",
            L"║    [====]    [####]    [====]      ║",
            L"║      ||        ||        ||        ║",
            L"║    [====]    [####]    [====]      ║",
            L"║         ⚔️  СУНДУК ⚔️               ║",
            L"║    🗝️                            🗝️ ║",
            L"╚════════════════════════════════════╝"
        };

        // Город
        locations["town"] = {
            L"╔════════════════════════════════════╗",
            L"║    🏘️ ГОРОД 🏘️                     ║",
            L"║    ┌───┐  ┌───┐  ┌───┐  ┌───┐     ║",
            L"║    │🏠 │  │🏪 │  │🏦 │  │🏥 │     ║",
            L"║    └───┘  └───┘  └───┘  └───┘     ║",
            L"║         ════╦════ ════            ║",
            L"║    ┌───┐  ═══╩═══  ┌───┐          ║",
            L"║    │🏨 │  ═══════  │🏛️ │          ║",
            L"║    └───┘          └───┘           ║",
            L"╚════════════════════════════════════╝"
        };
    }

    const std::vector<std::wstring>& GetLocation(const std::string& name) {
        return locations[name];
    }

    std::vector<std::string> GetLocationNames() {
        std::vector<std::string> names;
        for (const auto& pair : locations) {
            names.push_back(pair.first);
        }
        return names;
    }
};

// ИСПРАВЛЕНО: вспомогательная функция для конвертации string в wstring
std::wstring StringToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

int main() {
    // Создание рендерера
    ConsoleRenderer renderer(80, 25, true);
    renderer.SetTitle(L"Моя ASCII игра");

    // Создание менеджера локаций
    LocationManager locationManager;

    // Переменные для управления
    int currentLocationIndex = 0;
    auto locationNames = locationManager.GetLocationNames();
    int playerX = 40;
    int playerY = 12;

    // Основной игровой цикл
    bool running = true;

    while (running) {
        // Очистка экрана
        renderer.Clear(Color(Color::BLACK, Color::BLACK));

        // Получение текущей локации
        const auto& currentLocation = locationManager.GetLocation(locationNames[currentLocationIndex]);

        // Отрисовка локации через метод рендерера
        renderer.DrawLocation(5, 2, currentLocation);

        // Отрисовка игрока
        renderer.SetPixel(playerX, playerY, L'☺', Color(Color::BRIGHT_WHITE, Color::BLACK));

        // Отрисовка UI
        renderer.DrawBox(0, 0, 79, 24, L'█', Color(Color::WHITE, Color::BLUE));

        // ИСПРАВЛЕНО: правильное формирование строки
        std::wstring locationText = L"Локация: " + StringToWString(locationNames[currentLocationIndex]);
        renderer.DrawText(2, 1, locationText, Color(Color::BRIGHT_WHITE, Color::BLUE));

        // Информационная панель
        renderer.DrawFilledRect(0, 20, 79, 24, L' ', Color(Color::BLACK, Color::BLUE));
        renderer.DrawText(2, 21, L"WASD - движение | N - след. локация | P - пред. локация | ESC - выход",
            Color(Color::BRIGHT_WHITE, Color::BLUE));

        renderer.Render();

        // Обработка ввода
        int key = renderer.GetKeyPress();
        switch (key) {
        case 'w': case 'W': playerY = max(1, playerY - 1); break;
        case 's': case 'S': playerY = min(23, playerY + 1); break;
        case 'a': case 'A': playerX = max(1, playerX - 1); break;
        case 'd': case 'D': playerX = min(78, playerX + 1); break;
        case 'n': case 'N':
            currentLocationIndex = (currentLocationIndex + 1) % locationNames.size();
            break;
        case 'p': case 'P':
            currentLocationIndex = (currentLocationIndex - 1 + static_cast<int>(locationNames.size())) % static_cast<int>(locationNames.size());
            break;
        case 27: running = false; break; // ESC
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}