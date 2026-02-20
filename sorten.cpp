#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

const std::map<std::string, std::string> defaultRules = {
  {"./multimedia/images/@", "./*.{jpg,png,webp,gif,jpeg}"},
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
std::map<std::string, std::string> parseConfig(const std::string& path) {
    std::map<std::string, std::string> rules;
    std::ifstream file(path);
    if (!file.is_open()) return rules; // Default or empty

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::regex pair_regex(R"(\s*\"([^\"]+)\"\s*:\s*\"([^\"]+)\"\s*)");
    auto words_begin = std::sregex_iterator(content.begin(), content.end(), pair_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        rules[match[1].str()] = match[2].str();
    }
    return rules;
}

// Convert fast-glob string `*.{jpg,png}` or `*.apk` into a list of suffixes `.jpg`, `.png`, `.apk`
std::vector<std::string> extractExtensions(const std::string& globPat) {
    std::vector<std::string> exts;
    std::regex braced(R"(\.\/\*\.\{([^\}]+)\})");
    std::smatch match;
    if (std::regex_search(globPat, match, braced)) {
        std::string list = match[1].str();
        size_t start = 0;
        size_t end = list.find(',');
        while (end != std::string::npos) {
            exts.push_back("." + list.substr(start, end - start));
            start = end + 1;
            end = list.find(',', start);
        }
        exts.push_back("." + list.substr(start));
    } else {
        std::regex single(R"(\.\/\*\.(.+))");
        if (std::regex_search(globPat, match, single)) {
            exts.push_back("." + match[1].str());
        }
    }
    return exts;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() >= suffix.length()) {
        return (str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0);
    }
    return false;
}

int main(int argc, char* argv[]) {
    std::string configPath = "./config.json";

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
            configPath = argv[++i];
        }
    }

    std::map<std::string, std::string> rules;
    if (fs::exists(configPath)) {
        rules = parseConfig(configPath);
        if (rules.empty()) { // fallback if parsing failed or file was empty
            rules = defaultRules;
        }
    } else {
        rules = defaultRules;
    }

    // Map extensions to destination directories
    std::map<std::string, std::string> extToKey;
    for (const auto& pair : rules) {
        const std::string& key = pair.first;
        const std::string& glob_pat = pair.second;
        std::vector<std::string> exts = extractExtensions(glob_pat);
        for (const auto& ext : exts) {
            extToKey[ext] = key;
        }
    }

    // Iterate current directory
    for (const auto& entry : fs::directory_iterator(".")) {
        if (!entry.is_regular_file()) continue;

        std::string filename = entry.path().filename().string();
        std::string filepath = entry.path().string();
        
        // Skip dotfiles just like default fast-glob
        if (filename.front() == '.') continue;
        
        std::string bestMatchExt = "";
        std::string bestKey = "";

        // Find matching extension
        for (const auto& pair : extToKey) {
            const std::string& ext = pair.first;
            const std::string& key = pair.second;
            if (endsWith(filename, ext)) {
                // Prefer longer extension match (e.g. .tar.gz over .gz)
                if (ext.length() > bestMatchExt.length()) {
                    bestMatchExt = ext;
                    bestKey = key;
                }
            }
        }

        if (bestKey.empty()) continue; // No match

        std::string fsExt = entry.path().extension().string();
        std::string extension;

        if (fsExt == ".gz") {
            size_t dotPos = filename.find('.');
            if (dotPos != std::string::npos) {
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
        std::string destDir = bestKey;
        size_t atPos = destDir.find('@');
        if (atPos != std::string::npos) {
            destDir.replace(atPos, 1, extension);
        }

        fs::path destPath = fs::path(destDir) / filename;

        try {
            fs::create_directories(destPath.parent_path());
            
            // To ensure safe copying across devices if needed, standard practice is:
            // But std::filesystem::rename is faster and works inside same filesystem.
            // If it fails due to EXDEV, one would use copy() then remove(). 
            // In sorten-js `fsx.move` handles this. We will catch EXDEV and fallback to copy/remove.
            std::error_code ec;
            fs::rename(filepath, destPath, ec);
            if (ec) {
                // Fallback to copy/remove
                fs::copy(filepath, destPath, fs::copy_options::overwrite_existing, ec);
                if (!ec) {
                    fs::remove(filepath, ec);
                }
            }
            
            if (ec) {
                std::cerr << "[" << filename << "]: " << ec.message() << std::endl;
            } else {
                std::cout << filename << " moved successfully" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[" << filename << "]: " << e.what() << std::endl;
        }
    }

    return 0;
}
