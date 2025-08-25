## Webkit wallpaper
Use webkit to render webpages as your wallpaper on X11
### How to build
Clone the repo and make sure you have installed all the dependencies listed in `shell.nix` (or simply use nix-shell) then run
```
./build
```
### How to use
After the build phase, use the command like below:
```
./webwp --uri "file:///home/username/index.html"
```
Uri can be anything like `http://localhost:3000` or `https://google.com`
