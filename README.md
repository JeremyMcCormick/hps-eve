# hps-eve

## Building

Dependencies you will need to have installed:

- [ROOT](https://root.cern/install/) - tested with version 6.18.04
- [LCIO](https://github.com/jeffersonlab/hps-lcio) - use the HPS LCIO fork or tag v02-07-05, NOT the ILCSoft master

To enable automatic downloading and handling of detector data, the libxml2 and CURL libraries need to present.

Here are instructions for installing them on Ubuntu using the package manager:

```
sudo apt-get install libxml2-dev libcurl4-openssl-dev
```

For ROOT, see the [build from source instructions](https://root.cern/install/build_from_source/). You need to make sure that OpenGL is enabled and that the Eve library has been created once the build completes. The library `libEve.so` should be present in your ROOT library installation directory. If it is missing, then make sure the appropriate graphics packages are all installed on your machine so that OpenGL is enabled and rebuild it.

In the following instructions, it is assumed that software packages will be installed in the `/scratch` directory, but this could be replaced by any work directory on your machine where you have write access.

Once ROOT has been built, you should source the setup script before starting the build procedures:

```
. root/bin/thisroot.sh
```

The exact location of this script depends on where you installed ROOT.

If you do not have LCIO installed already, it can be built in the following way:

```
cd /scratch 
git clone https://github.com/jeffersonlab/hps-lcio.git
cd hps-lcio
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/scratch/hps-lcio/install ..
make install
```

Then to build hps-eve:

```
cd /scratch
git clone https://github.com/jeremymmccormick/hps-eve.git
cd hps-eve
mkdir build
cd build
cmake -DLCIO_DIR=/scratch/hps-eve/install ..
make install
```

You should now have a working hps-eve installation in the `install` directory.

## Running

You will always need to setup ROOT before running the app:

```
. root/install/bin/thisroot.sh # replace with path to your ROOT setup script
```

Then source the hps-eve environment setup script from your install dir:

```
. /scratch/hps-eve/install/bin/hps-eve-env.sh
```

Now the command `hps-eve` should be found on the command line.

Running without any arguments will show the command line usage:

```
$ ./install/bin/hps-eve
Usage: hps-eve [args] [LCIO files]
    -g [gdml file]
    -b [bY]
    -l [level]
    -e [exclude coll]
    -c [cache dir]
GDML file is required if curl and libxml2 were not enabled.
One or more LCIO files are required.
ERROR: Missing one or more LCIO files (provide as extra arguments)
```

The only mandatory arguments are a list of one or more input LCIO files.

The `-g` argument can be used to supply your own GDML geometry file (typically not needed).

The `-b` argument is used to specify a fixed B-field value for track propagation. For 2019 data, the value `1.034` can be used (notice the sign is flipped from the typical HPS convention).

The `-l` switch specifies a log level from 0 (no output) to 6 (very verbose output).

The `-e` argument can be used multiple times to specify names of data collections that should be completely ignored.

The `-c` argument specifies a cache dir for downloading detector files. (By default, the directory `.cache` will be created in your current working directory.)

Here is an example showing typical command line usage:

```
./install/bin/hps-eve -l 2 -b 1.034 -e HodoscopeHits -e TrackerHitsECal -e HodoscopePreprocessedHits events.slcio
```

This will run with log level 2 using a fixed B-field value of 1.034, excluding several collections, with data loaded from the file `events.slcio`.
