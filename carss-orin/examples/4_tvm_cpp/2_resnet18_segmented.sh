#!/bin/bash

#!/bin/bash

PERIOD_MS=$1

source /home/rubis/miniconda3/etc/profile.d/conda.sh

conda activate tvm

export LD_LIBRARY_PATH="/home/rubis/workspace/rt_gpu_sched_evaluation/carss-orin/cuMiddleWare/lib:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="/home/rubis/workspace/tvm-segment/build:$LD_LIBRARY_PATH"

./2_resnet18_segmented $PERIOD_MS