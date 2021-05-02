#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <vector>

#include <stdlib.h>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace std;

// Stuff that can be edited 
string tgt_missing = "You didn't include a target! :(\n";
string home = getenv("HOME");
string user = getenv("USER");

string config_dir = home + "/.config/aurtomatic/";
string config_fn = "settings.conf";

// Not used rn lol
// int config_len = 3;

// Stuff loaded from config
string out_dir = "";
string data_dir = "";
string repo_name = "";

bool proc(const string command) {
    int code = system(command.c_str());
    printf("Exit code: %i\n", code);
    if (code == 0) {
        return true;
    } else {
        return false;
    }
}

void ensure_dir(const string path) {
    if (!fs::is_directory(path)) {
        mkdir(path.c_str(), 0777);
    }
}

void loadconfig() {

    if (fs::is_directory(config_dir) && fs::is_regular_file(config_dir + config_fn)) {
        cout << "Loading config from " << config_dir + config_fn << "\n";

        string line;
        string lines[3];

        ifstream config;
        config.open(config_dir + config_fn);

        if (config.is_open()) {
            int i = 0;
            while (getline(config,line)) {
                lines[i] = line;
                i++;
            }
            config.close();
        } else {
            cerr << "Unable to open config file\n";
            exit(1);
        }

        cout << "Config lines: \n";
        for (int i = 0; i < 3; i++) {
            string thisline = lines[i];
            // cout << thisline << "\n";

            if (thisline.find("datadir") != string::npos) {
                data_dir = thisline.substr(thisline.find("=")+1, -1);
                cout << "Data dir is: " << data_dir << "\n";
            } else if (thisline.find("outdir") != string::npos) {
                out_dir = thisline.substr(thisline.find("=")+1, -1);
                cout << "Out dir is: " << out_dir << "\n";
            } else if (thisline.find("repon") != string::npos) {
                repo_name = thisline.substr(thisline.find("=")+1, -1);
                cout << "Repo name is: " << repo_name << "\n";
            }

        }

        ensure_dir(data_dir);
        ensure_dir(out_dir);

    } else {
        cout << "Seems like you haven't configured things yet. Let's do that\n"; 
        // guess cpp fs standard is still WIP
        // does this still work if the dir exists? guess so
        mkdir(config_dir.c_str(), 0777);
        
        cout << "Directory to hold package folders (full path): ";
        cin >> data_dir;
        
        cout << "Directory to hold repo (full path): ";
        cin >> out_dir;

        cout << "Repo name: ";
        cin >> repo_name;

        ofstream config;
        config.open(config_dir + config_fn);
        config << "datadir=" << data_dir << "\n";
        config << "outdir=" << out_dir << "\n";
        config << "repon=" << repo_name << "\n";
        config.close();

        cout << "Here are your configured settings\n";
        cout << "Package storage: " << data_dir << "\n";
        cout << "Output repo: " << out_dir << "\n";
        cout << "Repo name: " << repo_name << "\n";

    }
}

bool makepkg(const string target) {
    chdir(target.c_str());

    string thisline;
    ifstream dependsf(".SRCINFO");

    vector<string> depends;

    if (dependsf.is_open()) {
        while (getline(dependsf,thisline)) {
            if (thisline.find("depends = ") != string::npos) {
                string newd = thisline.erase(0,10);
                depends.push_back(newd);
            } else if (thisline.find("makedepends = ") != string::npos) {
                string newd = thisline.erase(0,14);
                depends.push_back(newd);
            }
        }
        dependsf.close();
    } else {
        cerr << "Failed to open .SRCINFO for " << target << "\n";
        exit(1);
    }

    for (string depend : depends) {
        //string depend = depends[i];
        cout << "Installing " + depend + "\n";
        bool res = proc("sudo pacman -Sy --needed --noconfirm " + depend);
        if (!res) {
            cerr << "Failed to install " + depend + " with pacman.\n";
            exit(1);
        }
    }

    bool res = proc("updpkgsums");
    if (!res) {
        cerr << "Could not update checksums for " << target << "\n";
        exit(1);
    }

    res = proc("makepkg -f");
    if (!res) {
        cerr << "Could not build package " << target << "\n";
        exit(1);
    }

    cout << "Done building " << target << "\n";

    vector<string> files;

    for (const auto & entry : fs::directory_iterator(".")) {
        string tp = entry.path();
        if (tp.find(target) != string::npos && tp.find("pkg") != string::npos) {
            files.push_back(tp);
        }
    }

    // we shouldn't have more than one
    fs::rename(files[0], out_dir + "/" + files[0]);

    chdir("../");
    return true;
}

bool updrepo() {
    chdir(out_dir.c_str());

    vector<string> files;
    for (const auto & entry : fs::directory_iterator(".")) {
        string tp = entry.path();
        if (tp.find("tar") != string::npos && tp.find("pkg") != string::npos) {
            files.push_back(tp);
        }
    }

    for (const auto & pkg : files) {
        bool code = proc("repo-add " + repo_name + ".db.tar.zst " + pkg);
        if (!code) {
            cout << "Failed while adding " << pkg << " to repo\n";
            exit(1);
        }
    }

    chdir("../");
    return true;
}

// Add package
bool add(const string target) {
    chdir(data_dir.c_str());
    if (fs::is_directory(target)) {
        cout << "Seems like we've already got " << target << ".\nDid you mean to update?\n";
        return false;
    } else {
        bool clone = proc("git clone https://aur.archlinux.org/" + target + ".git");
        if (clone) {
            // we got source
            bool done = makepkg(target);
            if (done) {
                bool repo = updrepo();
                if (repo) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else {
            cerr << "Failed to clone from AUR for " + target + "\n";
            exit(1);
        }
    }
    return false;
}

// Update all packages
bool update() {
    chdir(data_dir.c_str());

    return true;
}

// Stop serving package
bool remove(const string target) {
    chdir(data_dir.c_str());

    return true;
}

int main(int argc, char *argv[]) {
    // This func exits on 1 if failed.
    // (it creates non-existent config)
    loadconfig();
    if (argc == 1) {
        cout << "You need to give me instructions. See 'help'\n";
    } else {
        string command = argv[1];
        if (command == "add") {
            if (argc != 3) {
                cout << tgt_missing;
            } else { // this is probably even lazier than I realize
                string target = argv[2];
                bool res = add(target);
                if (res) {
                    cout << "Added " << target << endl;
                    return 0;
                } else {
                    return 1;
                }
            }
        } else if (command == "remove") {
            if (argc != 3) {
                cout << tgt_missing;
            } else { // this is probably even lazier than I realize
                string target = argv[2];
                bool res = remove(target);
                if (res) {
                    return 0;
                } else {
                    return 1;
                }
            }
        } else if (command == "update") {
            cout << "Todo\n";
        } else {
            // TODO: show some kind of help
            cout << "I'll write usage docs once I have a functional program.\n";
        }
    }
    return 0;
}