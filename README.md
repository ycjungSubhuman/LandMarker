# LandMarker

A simple GUI application for picking vertex index landmarks on 3D mesh.

## Build

```
git clone --recursive https://github.com/ycjungSubhuman/LandMarker.git

cd LandMarker && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../ && make -j8
```

## Usage

```
Usage: LandMarker [options] 

Optional arguments:
-h --help               shows help message and exits
-v --version            prints version information and exits
--mesh                  Path to the input .obj mesh file [required]
--landmark_initial      Path to initial landmark (optional)
--output                Path to output landmark file (optional. Default: output.txt) [default: "output.txt"]
```

Middle click on the mesh: Add landmark.

Push 'back' button to remove the latest landmark.
Push 'save' button to save the file to the path specified by '--output'

### output.txt

Number of lines == number of landmarks.
Each line specifies the vertex index (0-based)
