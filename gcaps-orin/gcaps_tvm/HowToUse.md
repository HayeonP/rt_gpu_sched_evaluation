# Build
```bash
mkdir build
cd build
mkdir timelog
cmake .. && make
```

# How to use
```bash
cd build
sudo ./gcaps_tvm -i 1 -d 3 # i: enable ioctl (gcaps) / d: experiment duration
```