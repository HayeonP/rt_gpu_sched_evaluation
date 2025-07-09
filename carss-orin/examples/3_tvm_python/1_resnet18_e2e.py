import os
import sys
import time
import tag_layer
from datetime import datetime
import tvm_utils
import tvm
from tvm import relax




######## TVM Configuration ########
image_sources = [
    "https://raw.githubusercontent.com/pytorch/hub/master/images/dog.jpg",
    "https://upload.wikimedia.org/wikipedia/commons/1/14/Gatto_europeo4.jpg",
    "https://upload.wikimedia.org/wikipedia/commons/thumb/3/37/African_Bush_Elephant.jpg/960px-African_Bush_Elephant.jpg",        
]
    
image_path_list = []
for source in image_sources:
    image_path = source.split('/')[-1]
    if not os.path.exists(image_path):
        import urllib.request
        urllib.request.urlretrieve(
            source, image_path)
    image_path_list.append(image_path)

dev = tvm.device("cuda", 0)
ex = tvm.runtime.load_module("resnet18.so")
    
params = tvm_utils.load_params("resnet18.bin")
vm = relax.VirtualMachine(ex, dev)

segment_runner = relax.SegmentRunner(vm)
with open("segments_info", "r") as f:
    segments_info = f.read()    
segments_length = segment_runner.load(segments_info)

gpu_params = [tvm.nd.array(p, dev) for p in params["main"]]
labels = tvm_utils.get_imagenet_labels()


########################


def now_time_only():
    now = datetime.now()
    return now.strftime("%H:%M:%S") + ".{:06d}".format(now.microsecond)

def tvm_task_set_input(image_path):
    print(image_path)
    orig_image, image_tensor = tvm_utils.preprocess_image(image_path)

    input = orig_image
    gpu_input = tvm.nd.array(image_tensor.astype("float32"), dev)
    
    segment_runner.set_input(gpu_input, *gpu_params)
    
    return

def tvm_task_run_segment(i):
    print("Run Segment", i)
    segment_runner.run(i)

    return

def tvm_task_get_output():
    gpu_output = segment_runner.get_output()
    output = gpu_output[0].copyto(tvm.cpu(0))
    
    print(tvm_utils.get_label(output))
    
    return

def tvm_task_e2e(image_path):
    orig_image, image_tensor = tvm_utils.preprocess_image(image_path)

    input = orig_image
    gpu_input = tvm.nd.array(image_tensor.astype("float32"), dev)
    
    segment_runner.set_input(gpu_input, *gpu_params)

    for i in range(segments_length):
        print("Run Segment", i)
        segment_runner.run(i)

    gpu_output = segment_runner.get_output()
    output = gpu_output[0].copyto(tvm.cpu(0))
    
    print(tvm_utils.get_label(output))
    
    return

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <period_ms>")
        sys.exit(1)
        
    period_ms = float(sys.argv[1])
    if period_ms <= 0:
        print("Period must be positive.")
        sys.exit(1)
        
    tagged_tvm_task_set_input = tag_layer.excl_tag_fn(tvm_task_set_input, "tvm_task_set_input", def_period=int(period_ms) * 1000)
    tagged_tvm_task_run_segment = tag_layer.excl_tag_fn(tvm_task_run_segment, "tvm_task_run_segment", def_period=int(period_ms) * 1000)
    tagged_tvm_task_get_output = tag_layer.excl_tag_fn(tvm_task_get_output, "tvm_task_run_get_output", def_period=int(period_ms) * 1000)
    tagged_tvm_task_e2e = tag_layer.excl_tag_fn(tvm_task_e2e, "tvm_task_e2e", def_period=int(period_ms) * 1000)
    
    
    i = 0
    while True:
        release_ts = now_time_only()
        start_time = time.time()
        job_dropped = False
        try:
            tagged_tvm_task_e2e(image_path_list[i%3])
        except Exception as e:
            print("ERROR:",e)
            job_dropped = True
        end_time = time.time()
        finish_ts = now_time_only()
        response_time = end_time - start_time

        # 한 줄에 release, finish, response time 출력
        print(f"[release: {release_ts}] [finish: {finish_ts}] [response: {response_time:.6f} sec]")

        sleep_time = (period_ms / 1000.0) - response_time
        if sleep_time > 0:
            time.sleep(sleep_time)
            
        i += 1
