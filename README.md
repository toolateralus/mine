# Mine "engine"

## Things you MUST know before building:

### Platform:
- This is not a linux-only library, but right now it's only setup to build on linux. For windows users, see [Building on windows](#building-on-windows).

### Update the submodules!
- Don't forget to use `git submodule update --recursive` to update submodules.

### Building & Running on Linux: 

- You must have `glfw`, `OpenGL`, `GLEW`, `assimp`, and `libdraco` installed on your system to build and link currently.

- To install any neccesary dependencies: 
```bash
git submodule init
git submodule update --recursive
sudo apt update
sudo apt install libglfw3-dev libglew-dev libassimp-dev libdraco-dev
```
- as listed at the top of this document, update your git submodules.

- use `make run` to build & run the project.
- we also have `make clean`, `make all`, and `make run_asan` for leak debugging.

please report any issues with this process!

### Building on windows: 
- I don't know how to compile this on windows but there's no reason it wouldn't be possible : just unfamiliar with how to do it with CMD or PowerShell.

- if you do figure out how to build & run on windows please
make a pull request with the adjusted `makefile` / `.ps1 script`
