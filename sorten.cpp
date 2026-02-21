#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <regex>

using namespace std;
namespace fs = filesystem;

const map<string, string> defaultRules = {
    {"./multimedia/images/@", "./*.{jpg,png,webp,gif,jpeg,svg}"},
    {"./multimedia/videos/@", "./*.{mp4,mov}"},
    {"./multimedia/audios/@", "./*.{mp3,m4a}"},
    {"./files/archives/@", "./*.{zip,rar,tr.gz,7z,tar,tar.gz}"},
    {"./files/codes/@", "./*.{xml,html,css,js,jsx,tsx,ts,sql,md,json}"},
    {"./files/documents/@", "./*.{pdf,xlxx,docx,docs}"},
    {"./files/applications/windows/@", "./*.{exe,msi}"},
    {"./files/applications/android/@", "./*.apk"},
    {"./files/applications/linux/debian/@", "./*.deb"},
    {"./files/applications/bootables/@", "./*.iso"},
    {"./files/transfers/@", "./*.torrent"},
    {"./others/@", "./*.{bak,txt}"}
};

// A very naive JSON parser for flat key-value string pairs
map<string, string> parseConfig(const string& path) {
    map<string, string> rules;
    ifstream file(path);
    if (!file.is_open()) return rules; // Default or empty

    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    regex pair_regex(R"(\s*\"([^\"]+)\"\s*:\s*\"([^\"]+)\"\s*)");
    auto words_begin = sregex_iterator(content.begin(), content.end(), pair_regex);
    auto words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; ++i) {
        smatch match = *i;
        rules[match[1].str()] = match[2].str();
    }
    return rules;
}

// Convert fast-glob string `*.{jpg,png}` or `*.apk` into a list of suffixes `.jpg`, `.png`, `.apk`
vector<string> extractExtensions(const string& globPat) {
    vector<string> exts;
    regex braced(R"(\.\/\*\.\{([^\}]+)\})");
    smatch match;
    if (regex_search(globPat, match, braced)) {
        string list = match[1].str();
        size_t start = 0;
        size_t end = list.find(',');
        while (end != string::npos) {
            exts.push_back("." + list.substr(start, end - start));
            start = end + 1;
            end = list.find(',', start);
        }
        exts.push_back("." + list.substr(start));
    } else {
        regex single(R"(\.\/\*\.(.+))");
        if (regex_search(globPat, match, single)) {
            exts.push_back("." + match[1].str());
        }
    }
    return exts;
}

bool endsWith(const string& str, const string& suffix) {
    if (str.length() >= suffix.length()) {
        return (str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0);
    }
    return false;
}

void printHelp() {
    cout << "Sorten - A CLI tool to sort files by extension\n\n";
    cout << "Usage:\n";
    cout << "  sorten run [path] [options]\n\n";
    cout << "Commands:\n";
    cout << "  run     Sort files in the specified directory (default: current directory '.')\n\n";
    cout << "Options:\n";
    cout << "  -c, --config <path>   Path to config.json (default: ./config.json)\n";
    cout << "  -h, --help            Show this help message\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 0;
    }

    string command = argv[1];
    if (command == "-h" || command == "--help") {
        printHelp();
        return 0;
    }

    if (command != "run") {
        cerr << "Unknown command: " << command << "\n\n";
        printHelp();
        return 1;
    }

    string targetPath = ".";
    string configPath = "./config.json";

    // Parse arguments after 'run'
    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
            configPath = argv[++i];
        } else if (arg[0] != '-') {
            targetPath = arg;
        }
    }

    map<string, string> rules;
    if (fs::exists(configPath)) {
        rules = parseConfig(configPath);
        if (rules.empty()) { // fallback if parsing failed or file was empty
            rules = defaultRules;
        }
    } else {
        rules = defaultRules;
    }

    // Map extensions to destination directories
    map<string, string> extToKey;
    for (const auto& pair : rules) {
        const string& key = pair.first;
        const string& glob_pat = pair.second;
        vector<string> exts = extractExtensions(glob_pat);
        for (const auto& ext : exts) {
            extToKey[ext] = key;
        }
    }

    // Iterate target directory
    if (!fs::exists(targetPath) || !fs::is_directory(targetPath)) {
        cerr << "Error: Target path '" << targetPath << "' is not a valid directory.\n";
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(targetPath)) {
        if (!entry.is_regular_file()) continue;

        string filename = entry.path().filename().string();
        string filepath = entry.path().string();
        
        // Skip dotfiles just like default fast-glob
        if (filename.front() == '.') continue;
        
        string bestMatchExt = "";
        string bestKey = "";

        // Find matching extension
        for (const auto& pair : extToKey) {
            const string& ext = pair.first;
            const string& key = pair.second;
            if (endsWith(filename, ext)) {
                // Prefer longer extension match (e.g. .tar.gz over .gz)
                if (ext.length() > bestMatchExt.length()) {
                    bestMatchExt = ext;
                    bestKey = key;
                }
            }
        }

        if (bestKey.empty()) continue; // No match

        string fsExt = entry.path().extension().string();
        string extension;

        if (fsExt == ".gz") {
            size_t dotPos = filename.find('.');
            if (dotPos != string::npos) {
                extension = filename.substr(dotPos + 1);
            } else {
                extension = fsExt.substr(1);
            }
        } else if (!fsExt.empty()) {
            extension = fsExt.substr(1);
        } else {
            // Default behaviour if no extension but matched somehow
            extension = "unknown";
        }

        // Replace "@" in key with the extension
        string destDir = bestKey;
        size_t atPos = destDir.find('@');
        if (atPos != string::npos) {
            destDir.replace(atPos, 1, extension);
        }

        fs::path destPath = fs::path(targetPath) / destDir / filename;

        try {
            fs::create_directories(destPath.parent_path());
            
            // To ensure safe copying across devices if needed, standard practice is:
            // But filesystem::rename is faster and works inside same filesystem.
            // If it fails due to EXDEV, one would use copy() then remove(). 
            // In sorten-js `fsx.move` handles this. We will catch EXDEV and fallback to copy/remove.
            error_code ec;
            fs::rename(filepath, destPath, ec);
            if (ec) {
                // Fallback to copy/remove
                fs::copy(filepath, destPath, fs::copy_options::overwrite_existing, ec);
                if (!ec) {
                    fs::remove(filepath, ec);
                }
            }
            
            if (ec) {
                cerr << "[" << filename << "]: " << ec.message() << endl;
            } else {
                cout << filename << " moved successfully" << endl;
            }
        } catch (const exception& e) {
            cerr << "[" << filename << "]: " << e.what() << endl;
        }
    }

    return 0;
}
