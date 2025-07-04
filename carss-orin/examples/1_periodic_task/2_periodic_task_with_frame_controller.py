import time
import sys
from tag_layer_fps import FrameController, frame_job_tag_fn

if len(sys.argv) != 2:
    print("Usage: python script.py <period_ms>")
    sys.exit(1)

period_ms = float(sys.argv[1])           # 주기(ms)
if period_ms <= 0:
    print("Period must be positive.")
    sys.exit(1)
fps = 1000.0 / period_ms                 # FPS로 변환

def dummy_job(idx):
    for _ in range(2000000):  # 300ms
        a = 1
        a = a + 1

try:
    fc = FrameController("dummy", fps, False)
    tagged_dummy_job = frame_job_tag_fn(dummy_job, fc, "dummy", False)
except Exception as e:
    print(f"Error initializing FrameController or registering job: {e}")
    sys.exit(1)

i = 0
while True:
    try:
        fc.frame_start()  # 프레임 시작

        start_time = time.time()
        job_dropped = False
        try:
            tagged_dummy_job(i)
        except Exception as e:
            print(f"[CARSS] Dummy job {i} dropped or failed: {e}")
            job_dropped = True
        end_time = time.time()
        response_time = end_time - start_time

        print(f"[CARSS] Dummy job {i} response time: {response_time:.6f} seconds | Dropped: {job_dropped} [{time.time()}]")
        if job_dropped:
            print(f"[INFO] Dummy job {i}: DROPPED")
        else:
            print(f"[INFO] Dummy job {i}: COMPLETED in {response_time:.6f} sec")

        fc.frame_end()  # 프레임 종료

        # 주기 맞추기 (optional, 필요시)
        sleep_time = (period_ms / 1000.0) - (end_time - start_time)
        if sleep_time > 0:
            time.sleep(sleep_time)

        i += 1
    except Exception as e:
        print(f"Loop exception: {e}")
        break
