# Mine "engine"

## Things you MUST know before building:

### Platform:
- This is not a linux-only library, but right now it's only setup to build on linux. For windows users, see [Building on windows](#building-on-windows).


### Update the submodules!
- Don't forget to use `git submodule update --recursive` to update submodules.

### Building & Running on Linux: 

- You must have `glfw`, `OpenGL`, `GLEW`, `assimp`, and `libdraco` installed on your system to build and link currently.

- To install these libraries on Linux using `apt`, you can run the following command:

```bash
sudo apt update
sudo apt install libglfw3-dev libglew-dev libassimp-dev libdraco-dev
```
- as listed at the top of this document, update your git submodules.

- then, you can just run `./build.sh r` or for a better incremental build, `make run` 
- we also have `make clean` and `make all`

please report any issues with this process!


### Debugging / Running the program from somewhere other than the project root: 

in `src/main.cpp`
```cpp
/*
 IMPORTANT : 
  if you intend to debug with RenderDoc or run the program from anywhere but the project root, you must adjust
  this RESOURCE_DIR_PATH to the correct path of the resources directory.
  
  I just use VScode to right click the folder, get 'copy path' and paste it here.
*/

static const std::string RESOURCE_DIR_PATH = "res";
```

### Building on windows: 
- I don't know how to compile this on windows but there's no reason it wouldn't be possible : just unfamiliar with how to do it with CMD or PowerShell.

- if you do figure out how to build & run on windows please
make a pull request with the adjusted `makefile` / `.ps1 script`