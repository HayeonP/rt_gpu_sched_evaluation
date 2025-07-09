# Test Environments
- Nvidia AGX Orin 64GB
- L4T R36.3
- CUDA 12.2

### Install L4T R36.3.0
* Access https://developer.nvidia.com/embedded/jetson-linux-r363
* Download followings

    (1) `Driver Package (BSP)`

    (2) `Sample Root Filesystem`

    (3) `Driver Package (BSP) Sources`

    (4) `Bootlin Toolchain gcc 11.3`

* Install essential packages
    ```bash
    sudo apt install wget lbzip2 build-essential bc zip libgmp-dev libmpfr-dev 
    sudo apt install libmpc-dev libncurses-dev flex bison libssl-dev qemu-user-static libxml2-utils
    ```

* Setup toolchains
    ```bash
    mkdir ~/l4t-gcc

    # Move and unzip the archive
    mv ~/Downloads/aarch64--glibc--stable-2022.08-1.tar.bz2 ~/l4t-gcc
    cd ~/l4t-gcc && tar xf aarch64--glibc--stable-2022.08-1.tar.bz2
    mv aarch64--glibc--stable-2022.08-1/* .
    rmdir aarch64--glibc--stable-2022.08-1/

    # Install essential packages to build the kernel
    sudo apt install wget lbzip2 build-essential bc zip libgmp-dev libmpfr-dev
    sudo pat install libmpc-dev libncurses-dev flex bison libssl-dev qemu-user-static libxml2-utils
    ```
* Setup BSP source codes
    ```bash
    # Create a build directory
    mkdir ~/build_dir
    
    # Move and unzip the archive
    mv ~/Downloads/public_sources.tbz2 ~/build_dir
    cd ~/build_dir && tar –xjvf public_sources.tbz2
    
    # Unzip kernel source code
    cd ~/build_dir/Linux_for_Tegra/source
    tar -xjvf kernel_src.tbz2
    tar xf kernel_oot_modules_src.tbz2
    tar xf nvidia_kernel_display_driver_source.tbz2
    ```

* Setup a flashing environment
    ```bash
   # Move the archive into the flash directory
   mv ~/Downloads/Jetson_Linux_R36.3.0_aarch64.tbz2 ~/flash_dir
   mv ~/Downloads/Tegra_Linux_Sample-Root-Filesystem_R36.3.0_aarch64.tbz2 ~/flash_dir
   #  Unzip BSP
   cd ~/flash_dir && tar -xjf Jetson_Linux_R36.3.0_aarch64.tbz2
   #  Unzip root filesystem
   cd Linux_for_Tegra/rootfs && sudo tar -jxpf ../../Tegra_Linux_Sample-Root-Filesystem_R36.3.0_aarch64.tbz2 -C .
   # Copy necessary drivers and libraries to the root file system
   cd .. && sudo ./apply_binaries.sh
    ```

### Build & Flash the kernel
* Build
    ```bash
    # Build the Linux kernel image
    cd ~/build_dir/Linux_for_Tegra/source
    export CROSS_COMPILE=~/l4t-gcc/bin/aarch64-buildroot-linux-gnu-
    make -C kernel
    
    # Install in-tree modules
    export INSTALL_MOD_PATH=~/flash_dir/Linux_for_Tegra/rootfs/
    sudo -E make install -C kernel
    
    # Install out-of-tree modules
    export IGNORE_PREEMPT_RT_PRESENCE=1
    export KERNEL_HEADERS=~/build_dir/Linux_for_Tegra/source/kernel/kernel-jammy-src
    make modules
    sudo -E make modules_install
    
    # Copy kernel image to the flash directory
    cp ~/build_dir/Linux_for_Tegra/source/kernel/kernel-jammy-src/arch/arm64/boot/Image ~/flash_dir/Linux_for_Tegra/kernel/Image
    ```

* Flash
    ```bash
    # Set username and password
    cd ~/flash_dir/Linux_for_Tegra
    sudo ./tools/l4t_create_default_user.sh -u <username> -p <password> -a -n <device_name> --accept-license
    
    # Flash kernel, RFS, and etc to the Orin
    sudo ./flash.sh jetson-agx-orin-devkit internal
    ```

### Setup Orin
* (Orin) Access to Orin via screen
    ``` bash
    # Install screen program for serial communication
    sudo apt-get install screen
    
    # Connect to Orin\'s terminal after 2~3 minutes (for waiting kernel is loaded)
    # It activates screen
    sudo screen /dev/ttyACM0 115200 # When serial device is ttyACM0
    
    # Enter username and password
    ```

* (Orin) Install networking packages
    ``` bash
    # Access to Wifi
    sudo nmcli device wifi connect <Wifi name> password <password>

    # Install netplan
    sudo apt update
    sudo apt install netplan.io
    ```

* (Orin) Setup netework configuration
    ``` bash
    # Modify netplan configuration
    sudo vi /etc/netplan/01-netcfg.yaml
    ```
    - Write the following configuration to the `01-netcfg.yaml`
        ```
        network:
            version: 2
            renderer: NetworkManager
            ethernets:
            eth0:
                dhcp4: no
                addresses:
                    - 192.168.0.11/24
                routes:
                    - to: default
                    via: 192.168.0.1
                nameservers:
                    addresses: [8.8.8.8, 8.8.4.4, 1.1.1.1]
        ```
* (Orin) Apply configuration
    ```bash
    sudo netplan apply

    # After now, you can access Orin via ssh
    ```

* (Orin) Install CUDA 
    ```bash
    sudo apt install nvidia-jetpack
    ```

* (Orin) Add CUDA configuration to `~/.bashrc`
    ```bash
    echo '# CUDA' >> ~/.bashrc
    echo 'export PATH="/usr/local/cuda/bin:$PATH"' >> ~/.bashrc
    echo 'export LD_LIBRARY_PATH="$CONDA_PREFIX/lib:/usr/local/cuda/lib64:$LD_LIBRARY_PATH"' >> ~/.bashrc
    echo 'export CMAKE_PREFIX_PATH="/usr/local/cuda:$CMAKE_PREFIX_PATH"' >> ~/.bashrc
    ```

* (Orin) Add GLIBCC path
    ```bash
    echo '# GLIBCC' >> ~/.bashrc
    echo 'export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH"' >> ~/.bashrc
    ```

### Set Orin to use 12 CPUs
* Change mode
    ```bash
    sudo /usr/sbin/nvpmodel -m 3
    ```