
# Access Control System

Access Control System is an ESP32-based project that provides RFID-based access management.  
It features Wi-Fi connectivity, Firebase integration for logging entries, and LCD display feedback.

## 📚 Project Structure

```
access_control_system/
├── main/
│   ├── CMakeLists.txt
│   ├── include/                    # Header files
│   │   ├── firebase.h
│   │   ├── lcd_display.h
│   │   ├── rfid.h
│   │   ├── wifi.h
│   │   ├── wifi_credentials.h       # Wi-Fi credentials (private)
│   │   └── firebase_credentials.h   # Firebase credentials (private)
│   ├── src/                         # Source files
│   │   ├── firebase.c
│   │   ├── lcd_display.c
│   │   ├── rfid.c
│   │   ├── wifi.c
│   │   └── main.c
├── components/                      # External components (e.g., rc522 RFID driver)
├── CMakeLists.txt                    # Project CMake
└── README.md                         # This file
```

## 🚀 Features

- **Wi-Fi Connectivity** — ESP32 connects to a predefined Wi-Fi network.
- **Time Synchronization** — Automatically syncs the system time via SNTP.
- **RFID Reader** — Detects RFID cards and identifies known UIDs.
- **LCD Display** — Displays access status (granted/denied/waiting).
- **Firebase Integration** — Logs access attempts (UID + timestamp) to Firebase Realtime Database.

## 🔧 Getting Started

### 1. Prerequisites

- ESP-IDF 5.x installed ([ESP-IDF Installation Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)).
- A Firebase Realtime Database project set up.
- Firebase API Key, Project ID, Email, and Password for authentication.
- An RC522 RFID module connected to ESP32.
- A SPI-based LCD display (e.g., ST7735S) connected to ESP32.

### 2. Setup Wi-Fi Credentials

Create `wifi_credentials.h` inside `main/include/`:

```c
#pragma once

#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"
```

✅ **Important**: Add `wifi_credentials.h` to `.gitignore`.

### 3. Setup Firebase Credentials

Create `firebase_credentials.h` inside `main/include/`:

```c
#pragma once

#define FIREBASE_API_KEY      "your-firebase-api-key"
#define FIREBASE_PROJECT_ID   "your-firebase-project-id"
#define FIREBASE_EMAIL        "your-firebase-email"
#define FIREBASE_PASSWORD     "your-firebase-password"
```

✅ **Important**: Add `firebase_credentials.h` to `.gitignore`.

### 4. Build and Flash

```bash
idf.py build
idf.py flash monitor
```

## 📡 Hardware Requirements

- ESP32 Dev Board
- RC522 RFID Module (SPI)
- LCD Display (e.g., ST7735S over SPI)
- Power supply

## 🖧 Wiring Diagram

### RC522 (RFID Reader)

| RC522 Pin     | ESP32 GPIO     |
|---------------|----------------|
| SDA (SS)      | GPIO 22         |
| SCK           | GPIO 18         |
| MOSI          | GPIO 23         |
| MISO          | GPIO 19         |
| IRQ           | Not connected   |
| GND           | GND             |
| RST           | GPIO 21         |
| 3.3V          | 3.3V            |

### LCD Display (ST7735S over SPI)

| LCD Pin       | ESP32 GPIO     |
|---------------|----------------|
| CS (Chip Sel) | GPIO 15         |
| RST           | GPIO 26         |
| DC (Data/Comm)| GPIO 27         |
| MOSI (DIN)    | GPIO 12         |
| SCK (CLK)     | GPIO 14         |
| LED (Backlight)| 3.3V           |
| VCC           | 3.3V            |
| GND           | GND             |


## 🚀 Future Improvements

- Add support for dynamic UID whitelist/blacklist updates via Firebase.
- Implement OLED screen option.
- Integrate push notification when access is granted/denied.
