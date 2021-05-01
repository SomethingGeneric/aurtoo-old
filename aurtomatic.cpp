#include <iostream>
#include <cstdlib>
#include <filesystem>

using namespace std;

// Stuff that can be edited 
string tgt_missing = "You didn't include a target! :(\n";
string config_dir = "~/.config/aurtomatic";
string config_fn = "settings.conf";

// Stuff loaded from config
string out_dir = "";
string data_dir = "";

bool proc(const string command) {
    int code = system(command.c_str());
    printf("Exit code: %i\n", code);
    if (code == 0) {
        return true;
    } else {
        return false;
    }
}

bool loadconfig() {



    return true;
}

bool add(const string target) {
    return true;
}

bool remove(const string target) {
    return true;
}

bool update() {
    return true;
}

int main(int argc, char *argv[]) {
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
            cout << "I'll write usage docs once I have a funcitonal program.\n";
        }
    }
    return 0;
}