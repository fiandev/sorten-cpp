# Sorten

A blazing-fast CLI tool built with C++ to sort files into categorized directories based on their extensions.

## Installation

You can install Sorten using the pre-compiled binaries available for your operating system.

### Linux

Download the binary and move it to a directory in your PATH (e.g., `/usr/local/bin`):

```bash
curl -L -o sorten https://github.com/fiandev/sorten-cpp/releases/latest/download/sorten-linux
chmod +x sorten
sudo mv sorten /usr/local/bin/
```

### macOS

Download the binary and move it to a directory in your PATH (e.g., `/usr/local/bin`):

```bash
curl -L -o sorten https://github.com/fiandev/sorten-cpp/releases/latest/download/sorten-macos
chmod +x sorten
sudo mv sorten /usr/local/bin/
```

### Windows

1. Download the `sorten-windows.exe` binary from the [Releases](https://github.com/fiandev/sorten-cpp/releases) page.
2. Rename the downloaded file to `sorten.exe`.
3. Move `sorten.exe` to a directory of your choice.
4. Add that directory to your system's `PATH` environment variable so you can run it from anywhere in the command prompt or PowerShell.

## Usage

Run the tool in the directory containing the files you want to sort:

```bash
sorten
```

### Options

- `-c, --config <path>`: Path to the `config.json` file (default: `./config.json`)

## Configuration

Create a `config.json` file in your working directory with sorting rules. Each key is the destination directory pattern, and the value is the glob pattern for matching files.

The `@` symbol in the destination will be replaced with the file extension.

Example `config.json`:

```json
{
  "./multimedia/images/@": "./*.{jpg,png,webp,gif,jpeg}",
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

## compile and run

```sh
make compile sorten.cpp
```

If no config file is found, it uses default rules similar to the above.

## License

MIT
