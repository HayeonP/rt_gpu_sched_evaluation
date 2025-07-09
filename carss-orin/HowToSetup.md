# Test Environments
- Nvidia AGX Orin 64GB
- L4T R36.3
- CUDA 12.2

### Install OpenGL
```bash
sudo apt install mesa-util freeglut3-dev libglu1-mesa-dev mesa-common-dev libglfw3-dev libglm-dev libgl1-mesa-dev
```

### Install curl
```bash
sudo apt-get install libcurl4-openssl-dev
```

### build
```bash
make
```

### Set library paths (NOTE: Set a workspace path correctly)
```bash
echo '# CARSS' >> ~/.bashrc
echo 'export <workspace path>/carss-orin/cuMiddleWare/lib:$LD_LIBRARY_PATH"' >> ~/.bashrc
echo 'export PYTHONPATH="<workspace path>>/carss-orin/cuMiddleWare/python:$PYTHONPATH"' >> ~/.bashrc
source ~/.bashrc
```

### Install opencv
