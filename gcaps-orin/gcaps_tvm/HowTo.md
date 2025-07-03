
* Install packages
```bash
sudo apt install librhash0 libcurl4-openssl-dev
conda ativate tvm
conda install -c conda-forge libstdcxx-ng=12
```

# 1. 빌드 디렉토리 생성
mkdir build && cd build

# 2. CMake 구성 (TVM 경로 설정 필요시)
cmake -DTVM_INCLUDE_DIRS=/path/to/tvm/include ..

# 3. 빌드
make -j$(nproc)

# 4. 실행 (taskset.csv 예제)
./tvm_inference -f taskset.csv -d 10