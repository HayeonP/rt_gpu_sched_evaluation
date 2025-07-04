# 1_periodic_task

- Task scheduling algorithm with a simple toy tasks (Task's WCET might be 150ms)
    ```bash
    # Execute scheduling middleware (CARSS)
    cd cuMiddlesWare
    sudo ./mid --policy rms     # Scheduling policies
                                # rms: Rate monotonic scheduling
                                # edf: Earliest-Deadline first
                                # fifo: First-In First-Out
                                # lst: Least slack time first

    # Execute tasks (in separate terminals)
    # Task 1: Period is 200ms
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 python3 1_periodic_task.py 200 

    # Task 2: Period is 500ms
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 python3 1_periodic_task.py 500 

    # Task 3: Period is 600ms
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 python3 1_periodic_task.py 600 

    # Task 4: Period is 700ms
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 python3 1_periodic_task.py 700 
    ```
- `2_periodic_task_with_frame_controller.py` will work identically. (But it uses the frame controller)
    ```bash
    # Execute scheduling middleware (CARSS)
    cd cuMiddlesWare
    sudo ./mid --policy rms     # Scheduling policies
                                # rms: Rate monotonic scheduling
                                # edf: Earliest-Deadline first
                                # fifo: First-In First-Out
                                # lst: Least slack time first

    # Execute tasks (in separate terminals)
    # Task 1: Period is 200ms
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 python3 2_periodic_task_with_frame_controller.py 200 

    # Task 2: Period is 500ms
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 python3 2_periodic_task_with_frame_controller.py 500 

    # Task 3: Period is 600ms
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 python3 2_periodic_task_with_frame_controller.py 600 

    # Task 4: Period is 700ms
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 python3 2_periodic_task_with_frame_controller.py 700 
    ```


- NOTE: CARSS provides 'non-preemptive uniprocessor scheduling'