# aurtoo
Tool to help maintain a personal repo of AUR packages

## Install
* From source: `makepkg -si`

## Usage
* Once you've installed, run `aurtoo help`
    * This will let you set up the folder to hold cloned AUR packages
    * As well as the folder to hold the built packages and the `<x>.db.tar.zst` file that is the local PacMan repo.
* To add a package from the AUR, simply run `aurtoo add <package_name>`
* For other commands, you can see `aurtoo help`

## Automation note
You can of course use a crontab or similar setup to automate `aurtoo update`, but please run `aurtoo add` manually, as you will have to authenticate with `sudo` to install depends for a package (if there are any)

## Use case
People who have multiple Arch machines, and want to only build AUR packages once.
By serving the `out_dir` folder via a web server, other Arch machines can be made to install the packages built by `aurtoo add`

## Caveat
Since I (the developer) run aurtoo exclusively on a server machine, aurtoo does not currently install the built package to the machine it's running on. Of course, you could simply configure the `out_dir` folder as a filesystem repo in your `pacman.conf` and then `pacman -Sy` the package after adding it via `aurtoo add`. If this is a feature that some people desire, I'd be happy to accept a PR :)
