import os, sys
import time
import tag_layer

print('Start')

if len(sys.argv) != 2:
    print("Usage: python script.py <period_ms>")
    sys.exit(1)

period_ms = float(sys.argv[1])           # 주기(ms)
if period_ms <= 0:
    print("Period must be positive.")
    sys.exit(1)

def dummy_job(idx):
    for _ in range(2000000):  # 300ms
        a = 1
        a = a + 1
# excl_tag_fn은 함수와 이름만 인자로 받음
tagged_dummy_job = tag_layer.excl_tag_fn(dummy_job, "dummy")

i = 0
while True:
    start_time = time.time()
    job_dropped = False
    try:
        tagged_dummy_job(i)
    except Exception as e:
        print(f"[CARSS] Dummy job {i} dropped or failed: {e}")
        job_dropped = True
    end_time = time.time()
    response_time = end_time - start_time

    print(f"[CARSS] Dummy job {i} response time: {response_time:.6f} seconds | Dropped: {job_dropped}")
    if job_dropped:
        print(f"[INFO] Dummy job {i}: DROPPED")
    else:
        print(f"[INFO] Dummy job {i}: COMPLETED in {response_time:.6f} sec")
    
    # 주기 맞추기 (optional, 필요시)
    sleep_time = (period_ms / 1000.0) - (end_time - start_time)
    if sleep_time > 0:
        time.sleep(sleep_time)
            
    i = i + 1
    
# 실행 통계 출력
# tagged_dummy_job.ts.print_exec_stats()
