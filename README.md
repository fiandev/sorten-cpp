<h1 align="center">📦 Sorten</h1>

<p align="center">
  A blazing-fast, cross-platform CLI tool built with C++ to organize and sort files into categorized directories based on their extensions.
</p>

<p align="center">
  <a href="https://github.com/fiandev/sorten-cpp/actions/workflows/release.yml"><img src="https://github.com/fiandev/sorten-cpp/actions/workflows/release.yml/badge.svg" alt="Auto Release"></a>
  <img src="https://img.shields.io/github/v/release/fiandev/sorten-cpp" alt="Latest Release">
  <img src="https://img.shields.io/github/license/fiandev/sorten-cpp" alt="License">
</p>

---

## ⚡ Features

- **Blazing Fast**: Written in modern C++ to ensure sub-millisecond file moving.
- **Cross-Platform**: Available natively for Linux, macOS, and Windows.
- **Customizable**: Sort by default rules or inject your own `config.json` rules.
- **Glob Support**: Use flexible glob patterns to match exact file behaviors.

---

## 🚀 Installation

You can install Sorten instantly by downloading the pre-compiled binaries for your operating system.

### 🐧 Linux

Download the binary and move it to a directory within your `PATH` (e.g., `/usr/local/bin`):

```bash
curl -L -o sorten https://github.com/fiandev/sorten-cpp/releases/latest/download/sorten-linux
chmod +x sorten
sudo mv sorten /usr/local/bin/
```

### 🍎 macOS

```bash
curl -L -o sorten https://github.com/fiandev/sorten-cpp/releases/latest/download/sorten-macos
chmod +x sorten
sudo mv sorten /usr/local/bin/
```

### 🪟 Windows

1. Download the `sorten-windows.exe` binary from the [Releases](https://github.com/fiandev/sorten-cpp/releases) page.
2. Rename the downloaded file to `sorten.exe`.
3. Add it to a directory included in your system's `PATH` environment variable.

---

## 💻 Usage

Sorten uses a straightforward command interface.

```bash
sorten run [path] [options]
```

To sort the current directory (`.`):

```bash
sorten run .
```

To sort a specific directory:

```bash
sorten run /path/to/my/messy/folder
```

### Options

| Flag           | Name        | Description                                                    |
| :------------- | :---------- | :------------------------------------------------------------- |
| `-c, --config` | Config Path | Path to a custom `config.json` file (default: `./config.json`) |
| `-h, --help`   | Help        | View usage and commands list                                   |

---

## ⚙️ Configuration

Sorten reads sorting rules from a `config.json` file. Each key acts as the **destination directory pattern**, and the value is the **glob pattern** for matching files.

> The `@` symbol in the destination directory key will be replaced with the exact file extension.

### Default Rules (Fallback)

If no `config.json` is provided, Sorten automatically applies the following default rules:

```json
{
  "./multimedia/images/@": "./*.{jpg,png,webp,gif,jpeg,svg}",
  "./multimedia/videos/@": "./*.{mp4,mov}",
  "./multimedia/audios/@": "./*.{mp3,m4a}",
  "./files/archives/@": "./*.{zip,rar,tar.gz,7z}",
  "./files/codes/@": "./*.{xml,html,css,js,jsx,tsx,ts,sql,md,json}",
  "./files/documents/@": "./*.{pdf,xlsx,docx,docs}",
  "./files/applications/windows/@": "./*.{exe,msi}",
  "./files/applications/android/@": "./*.apk",
  "./files/applications/linux/debian/@": "./*.deb",
  "./files/applications/bootables/@": "./*.iso",
  "./files/transfers/@": "./*.torrent",
  "./others/@": "./*.{bak,txt}"
}
```

---

## 🛠️ Building from Source

If you prefer to compile the application from the source code, you can use `make`. Make sure you have `g++` installed.

```bash
git clone https://github.com/fiandev/sorten-cpp.git
cd sorten-cpp

# Compile without running
make compile sorten.cpp
```
