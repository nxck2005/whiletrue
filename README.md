# whiletrue
![License](https://img.shields.io/github/license/nxck2005/whiletrue?color=green)
![Language](https://img.shields.io/github/languages/top/nxck2005/whiletrue?color=blue)
![C++23](https://img.shields.io/badge/std-C%2B%2B23-blue)
<!-- ![Repo Size](https://img.shields.io/github/repo-size/nxck2005/whiletrue) -->
![Status](https://img.shields.io/badge/status-active-success)
![Visitors](https://visitor-badge.laobi.icu/badge?page_id=nxck2005.whiletrue)

`whiletrue` or `Cybergrind` is a cyberpunk-themed terminal idle game, that you can run on the side while working. Made for my ADHD-self initially.

![Demo GIF](data/demo_gif.gif)

## Features

- **Pretty damn retro**: A retro-styled UI with dedicated windows for system status, terminal logs, and the black market.
- **Somewhat addicting**: Start by manually breaching (pressing space) and work your way up to.. no....
- **Persistent progress**: Your progress is automatically saved every 30 seconds and upon exiting the game, in the folder where you ran the build from. (save_data.json)

## Requirements

- **C++23 Compiler** (e.g., `g++ 12+`)
- **ncurses library**
- **pthread library**

## Building and Running

### Build

To compile the project and create the executable in the `build/` directory:

```bash
make
```

### Run

To build and launch the game immediately:

```bash
make run
```

### Clean

To remove build artifacts:

```bash
make clean
```

## Controls

| Key | Action |
|-----|--------|
| `Space` | Manually breach (Generate DATA) |
| `Up, Down arrow keys to select, then ENTER` | Purchase Quickhacks (Buildings) |
| `b` | Purchase Overclock Multiplier |
| `c` | Purchase DATA/SEC click share |
| `g` | Intercept Anomalous Signal (Golden Cache/Cookie) |
| `s` | Manual Save |
| `l` | Manual Load |
| `q` or `Esc` | Save and Quit |

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.
