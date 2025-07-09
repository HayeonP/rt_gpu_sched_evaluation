# 1_periodic_task

- Build
    ```bash
    build.sh
    ```

- Task scheduling algorithm with a simple toy tasks (Task's WCET might be 1.3sec)
    ```bash
    # Execute scheduling middleware (CARSS)
    cd cuMiddlesWare
    sudo ./mid --policy rms     # Scheduling policies
                                # rms: Rate monotonic scheduling
                                # edf: Earliest-Deadline first
                                # fifo: First-In First-Out
                                # lst: Least slack time first

    # Execute tasks (in separate terminals)
    # Task 1: Period is 2sec
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 1 ./1_periodic_task 2000

    # Task 2: Period is 3sec
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 2 ./1_periodic_task 3000

    # Task 3: Period is 4sec
    cd example/1_periodic_task
    sudo -E LD_LIBRARY_PATH=../../cuMiddleWare/lib -E PYTHONPATH=../../cuMiddleWare/python taskset -c 3 ./1_periodic_task 4000

    ```

- NOTE: CARSS provides 'non-preemptive uniprocessor scheduling'