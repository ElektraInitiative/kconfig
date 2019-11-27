# Elektra backend

## Running a program with the Elektra backend

(We have currently tested this in Arch Linux and OpenSUSE only)

**Prerequisite**

We assume that you already have the Qt5 libraries installed installed.
Apart from that you will need the following dependencies:

* CMake >= 3.5
* ECM >= 5.64.0
* Elektra >= 0.9 ([Installation guides](https://www.libelektra.org/installation/installation))

### Cloning and compiling the patched kconfig

```sh
git clone https://github.com/ElektraInitiative/kconfig.git kconfig_elektra
cd kconfig_elektra
mkdir build && cd build
cmake .. && make
```

### Running a program with the elektra backend

In this example we will use `kate`, but this should work with other programs too. 

Since we're in the `build` directory, the libraries are saved in the `bin` folder relative to where we are.
Running the following command should be enough:

```
LD_LIBRARY_PATH=$(pwd)/bin kate
```

If you're running this command from another directory don't forget to change the `$(pwd)/bin` to an absolute path.

### Verifying that elektra was used

At the time, elektra doesn't load the configurations from the files manually, so the default settings are generated and saved in elektra.
You can show those by running the following command in the terminal (you can replace yaml with json or ini)

```
kdb export user/sw/org/kde yaml
```


## Differences between Elektra config and .ini config

### Testing

Most of the tests check if the .ini backend specific files exist. 
These files are not created by the Elektra backend and the tests fail.

This is circumvented by ignoring the file checks, if KConfig has been linked against Elektra.

Not all tests have been patched yet.

 