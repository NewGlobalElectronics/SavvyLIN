# SavvyLIN

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**SavvyLIN** is an adaptation of the powerful [SavvyCAN](https://github.com/collin80/SavvyCAN) tool[reference:0], retargeted for the **Local Interconnect Network (LIN)** protocol. It is a Qt5-based, cross-platform application for loading, saving, capturing, visualizing, and reverse-engineering LIN bus frames[reference:1].

This project replaces the traditional CAN database (DBC) file support with **LIN Description File (LDF)** database support, making it a specialized tool for LIN network analysis and debugging. It is designed to work with New Global Electronics' proprietary hardware solution for LIN communication.

---

## ✨ Key Features

*   **LIN Protocol Support:** Dedicated to capturing, analyzing, and transmitting LIN bus frames[reference:2].
*   **LDF Database Support:** Uses LIN Description Files (LDF) to decode and interpret signals, replacing the DBC files used in CAN-based systems[reference:3].
*   **Cross-Platform:** Built with Qt5, SavvyLIN runs on Windows, macOS, and Linux[reference:4].
*   **Frame Capture & Logging:** Capture live LIN traffic and save it in various log formats[reference:5].
*   **Signal Visualization:** View and graph decoded LIN signals for in-depth analysis[reference:6].
*   **Frame Transmission:** Create, configure, and send custom LIN frames, either as one-off messages or periodically[reference:7].
*   **Frame Playback:** Replay recorded LIN traffic for testing and simulation purposes[reference:8].
*   **Hardware Agnostic:** Works with any Qt SerialBus driver, and is optimized for New Global Electronics' LIN hardware solution[reference:9].

---

## 🔄 Key Differences from SavvyCAN

| Feature | SavvyCAN (Original) | SavvyLIN (This Project) |
| :--- | :--- | :--- |
| **Protocol** | CAN (Controller Area Network)[reference:10] | LIN (Local Interconnect Network)[reference:11] |
| **Database** | DBC (CAN Database) files | LDF (LIN Description File) files[reference:12] |
| **Hardware** | Works with CAN interfaces (e.g., CANDue, PCAN, Vector)[reference:13] | Works with LIN interfaces, specifically New Global Electronics' hardware solution[reference:14] |

---

## 📦 Dependencies

*   **Qt 5.14.0 or higher**: Required for QtSerialBus and other modules[reference:15].
*   **QtSerialBus**: Essential for LIN communication support[reference:16].
*   **QCustomPlot**: Included in the source tree for plotting capabilities[reference:17].

---

## 🛠️ Compilation Instructions

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/NewGlobalElectronics/SavvyLIN.git
    cd SavvyLIN
