# TTYMirror

TTYMirror is a utility for mirroring data and control signals between two serial ports.

## Compilation

```
$ mkdir build
$ cd build
$ cmake ..
$ make 
```

## Usage

```
$ ./ttymirror -s source_port -m mirror_port [-b baudrate] [-d databits] [-p stopbits] [-y parity]
```
