#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <filesystem>
#include <queue>
#include <condition_variable>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/nvgpu.h>
#include <sys/mman.h>

#include <tvm/runtime/cpp_utils.h>
#include <tvm/runtime/cuda/cuda_common.h>
#include <support.h>
#include <tvm_utils.h>
#include <resnet18.h>
#include <debug.h>


#define WORKLOAD_RESNET18 0

using VmPtr = std::shared_ptr<tvm::runtime::relax_vm::VirtualMachineImpl>;

namespace fs = std::filesystem;

int task_identity;
mytimer_t mytimer;
std::chrono::time_point<std::chrono::system_clock> init_start_time;

struct Cpu {
    int id;
    float util;
};

std::string worklod_id_to_string(int workload_id){
    if(workload_id == WORKLOAD_RESNET18) {
        return "resnet18";
    }
    else{
        return "invalid";
    }
}

bool util_compare(Cpu c1, Cpu c2) { return c1.util < c2.util; }
bool RM_prio_compare(Task t1, Task t2) {
    if (t1.prio == 0 && t2.prio != 0) return false;
    else if (t1.prio != 0 && t2.prio == 0) return true;
    else return t1.period < t2.period;
}

std::vector<float> read_csv_column(std::string fn, int cid) {
    std::vector<float> ret;
    std::fstream fin(fn, std::ios::in);
    std::string line, word;
    int row = 0;
    while (std::getline(fin, line)) {
        std::stringstream s(line);
        row++;
        if (row == 1) continue; // skip header
        int index = 0;
        while (std::getline(s, word, ',')) {
            if (index == cid) ret.push_back(std::stof(word));
            index++;
        }
    }
    
    assert(!ret.empty());
    return ret;
}

std::vector<std::string> read_csv_column_as_string(std::string fn, int cid) {
    std::vector<std::string> ret;
    std::fstream fin(fn, std::ios::in);
    std::string line, word;
    int row = 0;
    while (std::getline(fin, line)) {
        std::stringstream s(line);
        row++;
        if (row == 1) continue; // skip header
        int index = 0;
        while (std::getline(s, word, ',')) {
            if (index == cid) ret.push_back(word);
            index++;
        }
    }
    assert(!ret.empty());
    return ret;
}

void load_taskset(std::vector<Task>& tasks, std::string filename) {
    if (filename.empty()) {
        printf("filename is empty! Skipped!\n");
        return;
    }

    std::vector<float> taskid_list = read_csv_column(filename, 0);

    std::vector<std::string> workload_str_list = read_csv_column_as_string(filename, 1);
    std::vector<int> workload_list;
    for(auto str : workload_str_list){
        if(str == "resnet18"){
            workload_list.push_back(WORKLOAD_RESNET18);
        }
        else{
            std::cout<<"[ERROR] Wrong workload: "<<str<<std::endl;
            exit(0);
        }
    }

    std::vector<float> period_list = read_csv_column(filename, 2);
    std::vector<float> wcet_list = read_csv_column(filename, 3);
    std::vector<float> util_list;
    for (size_t i = 0; i < taskid_list.size(); i++)
        util_list.push_back(wcet_list[i] * 100.0f / period_list[i]);
    for (size_t i = 0; i < taskid_list.size(); i++) {
        Task task;
        task.id = static_cast<int>(taskid_list[i]);
        task.prio = 1; // All tasks are real-time tasks
        task.workload_id = workload_list[i];
        task.period = static_cast<int>(period_list[i]);
        task.cpu_id = 0;
        task.util = util_list[i];
        tasks.push_back(task);
    }

    int ncpu = 12;
    std::vector<Cpu> cpus;
    for (int i = 0; i < ncpu; i++) {
        Cpu cpu;
        cpu.id = i + 1;
        cpu.util = 0;
        cpus.push_back(cpu);
    }
    
    for (size_t i = 0; i < tasks.size(); i++) {
        std::sort(cpus.begin(), cpus.end(), util_compare);
        cpus[0].util += tasks[i].util;
        tasks[i].cpu_id = cpus[0].id;
        printf("task %d (util=%.2f) is assigned to cpu %d (util=%.2f)\n",
               tasks[i].id, tasks[i].util, tasks[i].cpu_id, cpus[0].util);
    }

    int base_prio = 70;
    std::sort(tasks.begin(), tasks.end(), RM_prio_compare);
    for (size_t i = 0; i < tasks.size(); i++) {
        if (tasks[i].prio == 0) continue;
        
        tasks[i].prio = base_prio - i;
        std::cout << "task id: " << tasks[i].id << ", workload id: " << worklod_id_to_string(tasks[i].workload_id)
                  << ", prio: " << tasks[i].prio << std::endl;
    }
}

void prog_task(Task task, int duration, int fd, bool ioctl_enabled){

    const int sleep_time1 = 2500;
	const int sleep_time2 = 3000;

    task_identity = task.id;
    int period = task.period;
    int cpuid = task.cpu_id;
    int priority = task.prio;
    int nIter = duration * 1000 / period; // Duration: sec

    mytimer.init();
    
    // Waiting for timer initialization
    std::this_thread::sleep_until(init_start_time + std::chrono::milliseconds(sleep_time1));

    // Assign CPU
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpuid, &set);
    sched_setaffinity(gettid(), sizeof(cpu_set_t), &set);

    struct sched_param sched_params;
    sched_params.sched_priority = priority;
    int ret = sched_setscheduler(0, SCHED_FIFO, &sched_params); // real-time
    if (ret != 0) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }

    // another barrier
	std::this_thread::sleep_until(init_start_time + std::chrono::milliseconds(sleep_time2));
	// printf("Task %d sleeping until time2 done.\n", task.id);

    auto start_time = std::chrono::system_clock::now();
	auto next_time = init_start_time + std::chrono::milliseconds(sleep_time2);

    bool running = false;
    
    ResNet18 workload(task, fd, ioctl_enabled);

    while (nIter > 0) {
		std::this_thread::sleep_until(next_time);
		while (running == true);
		running = true;
		mytimer.record_start(next_time); 
		// printf("[%d:%d:%d] iteration # %d start.\n", getpid(), task.id, task.prio, nIter);

		workload.run();
		mytimer.record_stop();
		running = false;
		// printf("[%d:%d:%d] iteration # %d finished, elapsed: %f.\n", getpid(), task.id, task.prio, nIter, mytimer.elapsed_time);
		auto curr_time = std::chrono::system_clock::now();
		next_time += std::chrono::milliseconds(period);
		if (curr_time > next_time) {
			printf("Task %d job %d missed deadline!\n", task.id, nIter);
		}
		nIter--;
	}

	std::chrono::time_point<std::chrono::system_clock> exec_stop_time = std::chrono::system_clock::now();
	std::this_thread::sleep_until(exec_stop_time + std::chrono::milliseconds(1000));

    workload.finish();

    return;
}

int main(int argc, char* argv[]) {
    std::string filename = "../taskset.csv";
    int duration = 10;
    bool ioctl_enabled = false;
    int opt;
    while ((opt = getopt(argc, argv, "f:d:i:")) != -1) {
        switch (opt) {
            case 'f': filename = optarg; break;
            case 'd': duration = atoi(optarg); break;
            case 'i': ioctl_enabled = atoi(optarg); break;
            default: break;
        }
    }
    std::vector<Task> tasks;
    load_taskset(tasks, filename);

    int fd = open("/dev/nvgpu/igpu0/ctrl", O_RDWR);
	assert(fd >= 0);

    init_start_time = std::chrono::system_clock::now();

    // Create a counter for a barrier
    int *counter = (int*)mmap(nullptr, sizeof(int), 
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS,
                                -1, 0);
    assert(counter != MAP_FAILED);
    *counter = 0;

    // Start tasks
    int idx = 1;
	for (auto task : tasks) {
		int pid = fork();
		if (pid == 0) {
			prog_task(task, duration, fd, bool(ioctl_enabled));
            printf("[%d:%d], ", getpid(), task.id); mytimer.print_result();
            
            // The Barrier
            __sync_add_and_fetch(counter, 1);            
            while(__sync_val_compare_and_swap(counter, 0, 0) < int(tasks.size())) {
                usleep(100000);
            }

			// overhead_timer.print_result();
			return 0;
		}
		idx++;
	}

    int status = 0;
    while ((wait(&status)) > 0);
    return 0;
}
