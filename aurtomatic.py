#!/usr/bin/env python3

import os, sys, subprocess, shutil

from pathlib import Path

confdir = os.path.expanduser("~/.config/pacsync")

action = ""
config = {}
pkgs = []


def checkDir(path):
    if os.path.exists(path):
        return True
    else:
        return False


def ensureDir(path):
    if not checkDir(path):
        os.makedirs(path, exist_ok=True)
        return False
    else:
        return True


def setupConfig():
    print(
        "Hi there! Seems this is the first time you've run PacSync. Let's get you set up."
    )
    datadir = input("Where would you like to store package repos (from AUR): ")
    outdir = input("Where would you like the repo folder to be (for built packages): ")
    repon = input("Repo name: ")
    with open(confdir + "/settings.conf", "w") as f:
        f.write("datadir=" + datadir + "\n")
        f.write("outdir=" + outdir + "\n")
        f.write("repon=" + repon)
    ensureDir(datadir)
    ensureDir(outdir)


def loadConfig():
    if not ensureDir(confdir):
        setupConfig()
    with open(confdir + "/settings.conf") as f:
        lines = f.read().split("\n")
    for line in lines:
        if line[0] != "#" and "=" in line:
            pts = line.split("=")
            config[pts[0]] = pts[1]


def throwErr(label, text):
    print("Error: " + label + " \n" + text)
    with open(confdir + "/error.log", "a+") as f:
        f.write(
            "---------- " + label + "----------\n" + text + "--------------------\n"
        )


def listPkgs():
    os.chdir(config["datadir"])
    print("Checking all for updates")
    p = Path("./")
    pkgs = [f for f in p.iterdir() if f.is_dir()]
    return pkgs


def runProc(action):
    if " " in action:
        action = action.split(" ")
    else:
        action = [action]

    result = subprocess.run(action, stdout=subprocess.PIPE)
    return (result.returncode, result.stdout.decode("utf-8"))


def movePkg():
    pkgn = os.getcwd().split("/")[-1]

    files = os.listdir(".")
    tgt = ""
    for f in files:
        if pkgn in f and "pkg" in f:
            tgt = f

    (code, out) = runProc("mv " + tgt + " " + config["outdir"] + "/.")
    if code == 0:
        print("Package moved.")
    else:
        throwErr("mv for " + pkgn, out)


def updatePkg():
    pkgn = os.getcwd().split("/")[-1]
    print("Checking depends")
    (code, out) = runProc(
        "curl https://aur.archlinux.org/cgit/aur.git/plain/.SRCINFO?h=" + pkgn
    )
    if code == 0:
        if not "seems to be empty" in out:
            lines = out.split("\n")
            depends = []
            for line in lines:
                if "depends" in line:
                    depends.append(line.split("depends = ")[1])
                elif "makedepends" in line:
                    depends.append(line.split("makedepends = ")[1])
            for depend in depends:
                print("Installing: " + depend)
                (code, out) = runProc("sudo pacman -Syu " + depend)
                if code != 0:
                    throwErr("Installing " + depend + " while making " + pkgname, out)
            (code, out) = runProc("updpkgsums")
            if code == 0:
                print("Updated checksums. Making")
                (code, out) = runProc("makepkg -f")
                if code == 0:
                    print("Makepkg finished. Moving package to repo")
                    movePkg()
                else:
                    throwErr("makepkg for " + pkgn, out)
            else:
                throwErr("updpkgsums for " + pkgn, out)
        else:
            throwErr(
                "Dependency checking for " + pkgn,
                "Seems like this isn't an aur package",
            )
    else:
        throwErr(
            "Dependency checking for " + pkgn, "Seems like this isn't an aur package"
        )


def genRepo():
    os.chdir(config["outdir"])
    files = os.listdir(".")
    for f in files:
        if "pkg" in f:
            (code, out) = runProc("repo-add " + config["repon"] + ".db.tar.zst " + f)
            if code != 0:
                throwErr("Repo-add for " + config["repon"] + " on " + f, out)
    print("We need to decide how we'll config any remote-pushing")
    print("But for now we're done here.")


if len(sys.argv) == 1:
    action = "help"
else:
    action = sys.argv[1]
    loadConfig()

if action == "help":
    print("Help info here")
elif action == "update":
    pkgs = listPkgs()
    updated = 0
    for pkg in pkgs:
        print("Checking " + str(pkg))
        os.chdir(config["datadir"] + "/" + str(pkg))
        if checkDir(".git"):
            (code, out) = runProc("git pull")
            if code == 0:
                if "up to date" in out.lower():
                    print("No changes detected, not updating.")
                else:
                    print("Git pulled some changes. Updating.")
                    updatePkg()
                    updated += 1
                    print("Done updating " + pkg)
            else:
                print(pkg + " does not have a git repo. We're going to rebuild it.")
                updatePkg()
                updated += 1
    if updated != 0:
        print("At least one package updated, so we'll perform a repo sync (if setup)")
        syncRepo()
elif action == "add":
    if len(sys.argv) > 1:
        pkgn = sys.argv[2]
    else:
        pkgn = input("Package name: ")

    os.chdir(config["datadir"])
    if checkDir(pkgn):
        throwErr(
            "Adding package " + pkgn,
            "This package is already here. Did you mean to update or remove?",
        )
    else:
        try:
            os.system("git clone https://aur.archlinux.org/" + pkgn + ".git")
            print("We've added the source. Making.")
            os.chdir(pkgn)
            updatePkg()
            print("Updating repo")
            genRepo()
            print("All done.")
        except Exception as e:
            throwErr("Adding package " + pkgn, str(e))

elif action == "remove":
    if len(sys.argv) > 1:
        pkgn = sys.argv[2]
    else:
        pkgn = input("Package name: ")
    os.chdir(config["datadir"])
    if checkDir(pkgn):
        shutil.rmtree(pkgn)
        print("Removed " + pkgn + " from package store")
    else:
        print(pkgn + " not found in the store, moving on.")
    os.chdir(config["outdir"])
    print("Checking repo.")
    (code, out) = runProc("rm -rfv *" + pkgn + "*")
    if code == 0:
        print("Ensured " + pkgn + " is no longer in the repo")
        print("Updating repo to account for this.")
        genRepo()
    else:
        throwErr("While removing " + pkgn, out)
