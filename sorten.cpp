#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <regex>
#include <algorithm>   // for std::transform
#include <cctype>      // for tolower
#include <stdexcept>

using namespace std;
namespace fs = std::filesystem;

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

string toLower(const string& s) {
    string low = s;
    transform(low.begin(), low.end(), low.begin(), ::tolower);
    return low;
}

bool endsWith(const string& str, const string& suffix) {
    if (str.length() < suffix.length()) return false;
    string strEnd = str.substr(str.length() - suffix.length());
    return toLower(strEnd) == toLower(suffix);
}

map<string, string> parseConfig(const string& path) {
    map<string, string> rules;
    ifstream file(path);
    if (!file.is_open()) return rules;

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
    for (auto& e : exts) e = toLower(e);
    return exts;
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
        if (rules.empty()) {
            rules = defaultRules;
        }
    } else {
        rules = defaultRules;
    }

    // Build extension->key map
    map<string, string> extToKey;
    for (const auto& pair : rules) {
        const string& key = pair.first;
        const string& glob_pat = pair.second;
        auto exts = extractExtensions(glob_pat);
        for (const auto& ext : exts) {
            extToKey[ext] = key;
        }
    }

    // Target directory check
    if (!fs::exists(targetPath) || !fs::is_directory(targetPath)) {
        cerr << "Error: Target path '" << targetPath << "' is not a valid directory.\n";
        return 1;
    }

    cout << "Scanning directory: " << fs::absolute(targetPath) << endl;   // DEBUG

    // Collect moves
    using FileMove = pair<string, string>; // src, dst
    vector<FileMove> moves;

    try {
        for (const auto& entry : fs::directory_iterator(targetPath)) {
            if (!entry.is_regular_file()) continue;

            string filename = entry.path().filename().string();
            string filepath = entry.path().string();

            cout << "Checking file: " << filename << endl;   // DEBUG

            if (filename.front() == '.') continue;

            string bestMatchExt, bestKey;
            string lowFilename = toLower(filename);

            for (const auto& p : extToKey) {
                const string& ext = p.first;   // already lowercase
                const string& key = p.second;
                if (endsWith(lowFilename, ext)) {
                    if (ext.length() > bestMatchExt.length()) {
                        bestMatchExt = ext;
                        bestKey = key;
                    }
                }
            }

            if (bestKey.empty()) {
                cout << "  No rule matched for " << filename << endl;   // DEBUG
                continue;
            }

            // Determine extension for directory
            string extension;
            string fsExt = toLower(entry.path().extension().string());
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
                extension = "unknown";
            }
            extension = toLower(extension);

            string destDir = bestKey;
            size_t atPos = destDir.find('@');
            if (atPos != string::npos) {
                destDir.replace(atPos, 1, extension);
            }

            fs::path destPath = fs::path(targetPath) / destDir / filename;
            moves.emplace_back(filepath, destPath.string());
        }
    } catch (const exception& e) {
        cerr << "Error iterating directory: " << e.what() << endl;
        return 1;
    }

    cout << "Total files to move: " << moves.size() << endl;   // DEBUG

    if (moves.empty()) {
        cout << "No files matched the rules. Nothing to do." << endl;
        return 0;
    }

    // Perform moves
    for (const auto& [src, dst] : moves) {
        string filename = fs::path(src).filename().string();
        try {
            fs::create_directories(fs::path(dst).parent_path());
            error_code ec;
            fs::rename(src, dst, ec);
            if (ec) {
                fs::copy(src, dst, fs::copy_options::overwrite_existing, ec);
                if (!ec) {
                    fs::remove(src, ec);
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
