#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <conio.h> // For _kbhit() and _getch()
#include <vector>

std::mutex mtx;

void displayMarquee(const std::string& text, int width, int height, int refreshRateMs, int pollingRateMs) {
    int x = 0, y = 0;
    int dx = 1, dy = 1;
    std::size_t textLength = text.length(); // Use std::size_t to avoid conversion warning
    std::string command;
    std::vector<std::string> processedCommands;

    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    auto lastUpdateTime = std::chrono::steady_clock::now();
    auto lastRefreshTime = std::chrono::steady_clock::now();

    // Print the header once
    std::cout << "\033[2J\033[1;1H"; // Clear the screen
    std::cout << "************************************" << std::endl;
    std::cout << "* Displaying a marquee console! *" << std::endl;
    std::cout << "************************************" << std::endl;

    while (true) {
        auto currentTime = std::chrono::steady_clock::now();

        // Update position based on polling rate
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count() >= pollingRateMs) {
            x += dx;
            y += dy;

            // Check for collision with edges and reverse direction if necessary
            if (x <= 0 || x + textLength >= width) {
                dx = -dx;
            }
            if (y <= 0 || y >= height - 6) { // Adjust height to account for header and prompt
                dy = -dy;
            }

            lastUpdateTime = currentTime;
        }

        // Refresh marquee based on refresh rate
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastRefreshTime).count() >= refreshRateMs) {
            std::lock_guard<std::mutex> lock(mtx);

            // Move cursor to the start of the marquee area
            std::cout << "\033[4;1H"; // Move cursor to line 4, column 1

            // Clear the previous text
            for (int i = 0; i < height - 6; ++i) {
                std::cout << "\033[K\n"; // Clear the line and move to the next line
            }
            std::cout << "\033[K"; // Clear the last line
            std::cout << "\033[4;1H"; // Move cursor back to line 4, column 1

            // Print empty lines to move the text down
            for (int i = 0; i < y; ++i) {
                std::cout << "\n";
            }

            // Print spaces to move the text right
            for (int i = 0; i < x; ++i) {
                std::cout << " ";
            }

            // Print the text
            std::cout << text << std::flush;

            // Move cursor to the line below the marquee and print the prompt
            std::cout << "\033[" << (height - 1) << ";1H"; // Move cursor to the last line
            std::cout << "Enter a command for MARQUEE_CONSOLE: " << command << std::flush;

            // Print the processed commands
            for (const auto& cmd : processedCommands) {
                std::cout << "\nCommand processed in MARQUEE_CONSOLE: " << cmd << std::flush;
            }

            lastRefreshTime = currentTime;
        }

        // Check for user input
        if (_kbhit()) {
            char ch = _getch();
            if (ch == '\r') { // Enter key
                processedCommands.push_back(command); // Store the command
                command.clear(); // Clear the command after processing
            }
            else if (ch == '\b') { // Backspace key
                if (!command.empty()) {
                    command.pop_back();
                }
            }
            else {
                command += ch;
            }
        }

        // Sleep for a short duration to prevent high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    std::string text = "Hello World!";
    int width = 80; // Width of the display window
    int height = 24; // Height of the display window
    int refreshRateMs = 10; // Refresh rate in milliseconds
    int pollingRateMs = 20; // Polling rate in milliseconds

    displayMarquee(text, width, height, refreshRateMs, pollingRateMs);

    return 0;
}
