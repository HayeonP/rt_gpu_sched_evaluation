### Build
```bash
sh build.sh
```

### Workloads
* `1_resnet18_e2e.cpp`: Run resnet18 with an e2e range tag
```bash
sudo ./1_resnet18_e2e.sh <period_ms>
```

* `2_resnet18_segmented.cpp`: Run resnet18 with segement-level tag
```bash
sudo ./2_resnet18_segmented.sh <period_ms>
```