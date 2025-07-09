import os
import sys
import time
import tag_layer
from datetime import datetime

print('Start')

if len(sys.argv) != 2:
    print("Usage: python script.py <period_ms>")
    sys.exit(1)

period_ms = float(sys.argv[1])
if period_ms <= 0:
    print("Period must be positive.")
    sys.exit(1)

def now_time_only():
    now = datetime.now()
    return now.strftime("%H:%M:%S") + ".{:06d}".format(now.microsecond)

def dummy_job(idx):
    for _ in range(20000000):
        a = 1
        a = a + 1

tagged_dummy_job = tag_layer.excl_tag_fn(dummy_job, "dummy", def_period=int(period_ms) * 1000)

i = 0
while True:
    release_ts = now_time_only()
    start_time = time.time()
    job_dropped = False
    try:
        tagged_dummy_job(i)
        # dummy_job(i)
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
