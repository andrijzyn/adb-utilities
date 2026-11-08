#include <array>
#include <cstdio>
#include <curses.h>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <vector>

// Constants zone
constexpr size_t MAX_RETURN_SIZE = 1024;
const std::string SUCCESS_REMOVED = "Success";
const std::string SUCCESS_LACK = "Failure [not installed for 0]";
const std::string DEVICES_LACK = "adb: no devices/emulators found";

// Enum to represent menu options with a smaller base type (std::uint8_t)
enum class MenuOption : uint8_t { GUIDE, LISTPKG, CLEAN, EXIT };

// Package list
// Package list
const std::vector<std::string> PACKAGES = {
    "com.samsung.android.smartswitchassistant",  // Samsung Smart Switch Assistant
    "com.samsung.android.themestore",            // Samsung Theme Store
    "com.samsung.android.game.gametools",        // Game Tools
    "com.samsung.android.game.gamehome",         // Game Launcher
    "com.samsung.android.kidsinstaller",         // Samsung Kids Installer
    "com.samsung.android.app.appsedge",          // Apps Edge
    "com.samsung.android.app.updatecenter",      // Update Center
    "com.samsung.android.shortcutbackupservice", // Shortcut Backup Service
    "com.samsung.android.scloud",                // Samsung Cloud
    // "com.samsung.android.app.sharelive",      // Quick Share
    // "com.samsung.android.dialer",             // KEEP: Phone, no replacement installed
    "com.samsung.android.messaging",             // Samsung Messages (set Google Messages as default SMS first)
    // "com.samsung.android.app.contacts",       // KEEP: Contacts, no replacement installed
    "com.samsung.sree",                          // Samsung Services
    "com.samsung.android.app.tips",              // Samsung Tips
    "com.samsung.android.app.parentalcare",      // Parental Control
    "com.samsung.android.aremoji",               // Samsung AR Emoji
    // "com.sec.android.app.samsungapps",        // KEEP: Galaxy Store (updates for Samsung apps)
    "com.sec.android.easyMover.Agent",           // Smart Switch Agent
    "com.sec.android.app.chromecustomizations",  // Chrome Customizations
    "com.android.chrome",                        // Google Chrome
    "tv.sweet.player",                           // Sweet TV Player
    "com.megogo.application",                    // MEGOGO
    "com.netflix.partner.activation",            // Netflix Activation
    "com.microsoft.skydrive",                    // OneDrive
    "com.facebook.services",                     // Facebook Services
    "com.facebook.katana",                       // Facebook App
    "com.facebook.appmanager",                   // Facebook App Manager
    "com.facebook.system",                       // Facebook System Services
    "com.einnovation.temu",                      // Temu Shopping App
    "com.google.android.apps.tachyon",           // Google Duo/Meet
    "com.netflix.mediaclient",                   // Netflix App
    "com.scopely.monopolygo",                    // Monopoly GO! Game
    // "com.samsung.android.bbc.bbcagent",       // CHECK: Knox-related, Secure Folder in use
    "com.samsung.android.privateshare",          // Samsung Private Share
    "com.aura.oobe.samsung.gl",                  // AppCloud

    // --- Added: telemetry & marketing ---
    "com.sec.android.diagmonagent",              // Samsung diagnostics / telemetry
    "com.samsung.android.dqagent",               // Device Quality Agent
    "com.samsung.android.rubin.app",             // Customization Service (personalization)
    "com.samsung.android.ipsgeofence",           // Samsung Visit In (geofencing promos)
    "com.samsung.android.voc",                   // Samsung Members
    "com.google.android.feedback",               // Google Feedback

    // --- Added: AR Emoji & stickers ---
    "com.samsung.android.aremojieditor",         // AR Emoji Editor
    "com.sec.android.mimage.avatarstickers",     // AR Emoji stickers
    "com.samsung.android.app.camera.sticker.facearavatar.preload", // Camera AR stickers

    // --- Added: Bixby (only if unused) ---
    "com.samsung.android.bixby.agent",           // Bixby Voice
    "com.samsung.android.bixby.wakeup",          // "Hi, Bixby" wake-up
    "com.samsung.android.visionintelligence",    // Bixby Vision

    // --- Added: no matching device / service ---
    "com.microsoft.appmanager",                  // Link to Windows (Windows PC only)
    "com.samsung.android.app.watchmanagerstub",  // Galaxy Wearable stub (no Galaxy Watch)
    "com.samsung.android.carkey",                // Digital Car Key
    "com.samsung.android.spayfw",                // Samsung Pay framework (Google Wallet used)
    "com.sec.android.easyMover",                 // Smart Switch (one-time migration)
    "com.google.android.videos",                 // Google TV

    // --- Added: optional ---
    "com.hiya.star",                             // Caller ID & spam protection in Samsung Phone
    // "com.samsung.android.widget.pictureframe",   // Picture frame widget
    "com.sec.android.widgetapp.easymodecontactswidget", // Easy Mode contacts widget
    "com.google.android.printservice.recommendation",   // Print service recommendations
    "com.android.dreams.phototable",             // Photo screensaver
    "com.android.egg",                           // Android easter egg
};

// Execute a command and get its output
inline std::optional<std::string> exec(const std::string &cmd) {
    std::array<char, MAX_RETURN_SIZE> buffer{};
    std::string result;

    const std::unique_ptr<FILE, int (*)(FILE *)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return std::nullopt;
    }

    // Read data from the pipe and accumulate it into the result string
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

inline void waitForKeyPress(const std::string &message = "Press any key to return to menu...") {
    mvprintw(4, 2, "%s", message.c_str());
    getch();
    clear();
}

// Check if adb devices are available
inline bool checkDevicesAvailable() {
    std::optional<std::string> responseOpt = exec("adb devices");

    // Verify the command execution result
    if (!responseOpt) {
        return false;
    }

    const std::string &response = *responseOpt;
    std::istringstream responseStream(response);
    std::string line;
    int lineCount = 0;

    // Count lines in response
    while (std::getline(responseStream, line)) {
        if (!line.empty()) {
            ++lineCount;
        }
    }

    // If less than two lines, the device is not connected
    if (lineCount < 2) { // The first line is the heading
        spdlog::critical("No devices or emulators found. Please connect a device.");
        return false; // No devices found
    }

    spdlog::info("Devices detected:\n{}", response);
    return true; // Devices are available
}

inline void guideBook() {
    mvprintw(1, 2, "How use? (press ENTER to continue)");
    getch();
    mvprintw(2, 2, "Firstly make sure that your computer has installed ADB (Android Debug Bridge)");
    getch();
    mvprintw(2, 2, "In smartphone go to Developer mode by pressing 7 times on the build number in 'About device'");
    getch();
    clear();
    mvprintw(1, 2, "How use? (press ENTER to continue)");
    mvprintw(2, 2, "Press Enable debugging via USB");
    getch();
    mvprintw(2, 2, "Connect your Android device to your computer using USB cable");
    getch();
}

// Proceed with package removal if devices are found
inline void removeRequest() {
    for (const std::string &package : PACKAGES) {
        // Execute the adb uninstall command
        auto responseOpt = exec("adb shell pm uninstall --user 0 " + package);

        // Check if we received a valid response
        if (responseOpt) {
            const std::string &response = *responseOpt;

            // If package successfully has been removed
            if (response.find(SUCCESS_REMOVED) != std::string::npos) {
                spdlog::info("Package {} has been removed successfully.", package);
            }
            // If package hasn't been removed
            else if (response.find(SUCCESS_LACK) != std::string::npos) {
                spdlog::warn("Package {} is not present.", package);
            }
            // Unexpected response
            else {
                spdlog::critical("Unexpected response for package {}: {}", package, response);
            }
        } else {
            spdlog::error("Failed to execute adb command for package: {}", package);
        }
    }
}

inline void listPackages() {
    auto packages = exec("adb shell pm list packages");
    if (packages) {
        spdlog::info("Packages found:\n{}", *packages);
    } else {
        spdlog::error("Failed to retrieve package list.");
    }
}


// Function to display the menu and get user choice
MenuOption displayMenu(const std::vector<std::string> &options) {
    int highlight = 0;
    int choice = -1; // To store the user choice

    while (choice == -1) {
        clear(); // Clear screen

        // Display menu options
        for (int i = 0; i < static_cast<int>(options.size()); ++i) {
            if (i == highlight) {
                attron(A_REVERSE);
            } // Highlight selected option
            mvprintw(i + 1, 2, "%s", options[i].c_str());
            if (i == highlight) {
                attroff(A_REVERSE);
            }
        }

        // Handle key press
        const int key = getch();
        switch (key) {
        case KEY_UP:
            highlight = (highlight == 0) ? options.size() - 1 : highlight - 1;
            break;
        case KEY_DOWN:
            highlight = (highlight + 1) % options.size();
            break;
        case '\n': // Enter key
            choice = highlight;
            break;
        default:
            break;
        }
    }

    return static_cast<MenuOption>(choice);
}

// Define functional points in menu + MenuOptions
inline bool menuLoop(const std::vector<std::string> &options) {
    const MenuOption choice = displayMenu(options);
    clear(); // Clear screen after option selection

    switch (choice) {
    case MenuOption::GUIDE:
        guideBook();
        break;
    case MenuOption::LISTPKG:
        if (checkDevicesAvailable()) {
            listPackages();
            waitForKeyPress(); // Wait for the user to press a key before returning to the menu
        }
        break;
    case MenuOption::CLEAN:
        if (checkDevicesAvailable()) {
            removeRequest();
            waitForKeyPress(); // Wait for the user to press a key before returning to the menu
        }
        break;
    case MenuOption::EXIT:
        return false;
    }

    return true;
}


int main() {
    // Initialize spdlog (console logging with default level)
    spdlog::set_level(spdlog::level::info); // Set minimum log level to info

    initscr();            // Initialize ncurses
    cbreak();             // Disable input buffering
    noecho();             // Disable echoing typed characters
    keypad(stdscr, true); // Enable arrow keys

    // Define menu options
    const std::vector<std::string> options = {"Action 1: How use?",
                                              "Action 2: List packages",
                                              "Action 3: Clean device",
                                              "Exit"};

    while (menuLoop(options)) {
    } // Start menu loop

    endwin(); // End ncurses session
    return 0;
}
