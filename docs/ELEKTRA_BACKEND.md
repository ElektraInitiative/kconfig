# Elektra backend

Elektra is a modular configuration library that can be used to integrate applications into a global key-value database.
The Elektra backend is intended to replace the INI Backend by passing the parsing and saving configurations responsibility to the Elektra library. 

## Running a program with the Elektra backend

We have currently tested this in Arch Linux (release 2019.11.01) and OpenSUSE (tumbleweed 84.87.20191025) only.

**Prerequisite**

We assume that you already have the Base and XML Modules from Qt5 installed.
Apart from that you will need the following dependencies:

* CMake >= 3.5
* ECM (extra-cmake-modules) >= 5.64.0
* Elektra = 0.9.1 ([Installation guides](https://www.libelektra.org/installation/installation))

### Cloning and compiling the patched kconfig

```sh
git clone https://github.com/ElektraInitiative/kconfig.git kconfig_elektra
cd kconfig_elektra
mkdir build && cd build
cmake .. && make
```

### Running a program with the elektra backend

We currently tested `kate`. 

Since we're in the `build` directory, the libraries are saved in the `bin` folder relative to where we are.
Running the following command should be enough:

```
LD_LIBRARY_PATH=$(pwd)/bin kate
```

If you're running this command from another directory don't forget to change the `$(pwd)/bin` to an absolute path.

### Verifying that Elektra was used

At the time, Elektra doesn't load the configurations from the files manually, so the default settings are generated and saved in elektra.

If you have the [elektra-qt-editor](https://www.libelektra.org/tools/qt-gui#elektra-qt-gui) installed you can access these configurations via a graphical user interface.
In the editor you will be able to traverse the configurations in a hierarchical structure. You can also modify, add and remove keys as you wish from there.

You can also use the kdb tool which is included in Elektra.

Run the following command from the terminal to show all the keys saved from the kde aplications (you can replace yaml with json or ini).

```
kdb export user/sw/org/kde yaml
```

You can get specific values by using `kdb get`:

```
kdb get "user/sw/org/kde/kate/#19/current/KTextEditor Renderer/Show Indentation Lines"
```

And you can also set that key to another value:

```
kdb set "user/sw/org/kde/kate/#19/current/KTextEditor Renderer/Show Indentation Lines" true
```

## Differences between Elektra and .ini Backends

### Limitations

- Only tested with `kate` and `dolphin`
- Configurations with absolute paths are still loaded using the INI backend

### Tests

Most of the tests check if the .ini backend specific files exist. 
These files are not created by the Elektra backend and the tests fail.

This is circumvented by ignoring the file checks, if KConfig has been linked against Elektra.

Not all tests have been patched yet.

 
