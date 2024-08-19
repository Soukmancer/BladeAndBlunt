This is a CommonLib implementation of a .NETScriptFramework plugin originally written by KernalsEgg. 
Much of the credit for the RE work goes to KernalsEgg.

`Override for new armor rating scaling formula`    

## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2019](https://visualstudio.microsoft.com/)
	* Desktop development with C++

## Register Visual Studio as a Generator
* Open `x64 Native Tools Command Prompt`
* Run `cmake`
* Close the cmd window

## Building
```
git clone https://github.com/colinswrath/BladeAndBlunt
cd ExamplePlugin-CommonLibSSE
git submodule update --init --recursive
cmake -B build -S .
```
