# LowkeyType

**LowkeyType** is a feature-rich, console-based typing test game written in C.  
It supports user profiles, multiple game modes, persistent statistics, and a colorful, user-friendly interface.

---

## Features

- **User Profiles:** Persistent stats, best scores, and progress tracking.
- **Endurance Mode:** Type as long as you can while maintaining accuracy and speed.
- **Raw Speed Mode:** Timed typing tests with customizable word count and difficulty.
- **Leaderboard:** Compare your performance with other users.
- **Profile View:** See your stats and skill assessment.
- **Dynamic Word Lists:** Loads words from external files for each difficulty.
- **ASCII Art Title Screen:** Customizable and colorful welcome screen.
- **Cross-Platform:** Works on Windows and Unix-like systems.
- **Color Output:** Color-coded feedback for mistakes and achievements.
- **Backspace Support:** Correct mistakes as you type.
- **Input Validation:** Robust handling of user input.
- **Console Width Detection:** Adapts output for better readability.

---

## Getting Started

### Prerequisites

- **C Compiler:** GCC, Clang, or MSVC
- **Platform:** Windows or Unix-like (Linux, macOS)
- **Terminal:** Supports ANSI colors (most modern terminals do)

### Files Needed

- `LowkeyType.c` (main source code)
- `users.txt` (auto-created for user profiles)
- `wordbaseL.txt`, `wordbaseM.txt`, `wordbaseH.txt` (word lists for each difficulty)
- `title.txt` (ASCII art for the title screen)

### Compilation

**On Windows (using GCC):**
```sh
gcc LowkeyType.c -o LowkeyType
```

**On Linux/macOS:**
```sh
gcc LowkeyType.c -o LowkeyType
```

### Running

```sh
./LowkeyType
```
or on Windows:
```sh
LowkeyType.exe
```

---

## Usage

1. **Start the program** and enter your username (no spaces).
2. **Choose a mode** from the main menu:
   - Endurance Mode
   - Raw Speed Mode
   - Leaderboard
   - Profile
   - Exit
3. **Follow on-screen instructions** for each mode.
4. **Your stats are saved** automatically.

---

## Customization

- **Word Lists:** Edit `wordbaseL.txt`, `wordbaseM.txt`, and `wordbaseH.txt` to add/remove words.
- **ASCII Art:** Replace or edit `title.txt` for a custom title screen.

---

## Example Screenshots

*(Add screenshots here if you wish!)*

---

## Author

Rotimi Dayo  
ACS130 Introduction to Systems Engineering and Software  
April 2025

---

## License

MIT License (recommended, but you can choose your own)

---

## Contributions

Pull requests and suggestions are welcome!
