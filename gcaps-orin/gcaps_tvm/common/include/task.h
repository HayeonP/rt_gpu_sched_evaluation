struct Task {
    int id;
    int workload_id; // 이미지 ID로 대체 가능 (여기서는 0: dog.jpg, 1: Gatto_europeo4.jpg)
    int period;     // ms 단위
    int cpu_id;     // CPU affinity
    int prio;       // 우선순위 (0: BE, 1: 실시간)
    float util;     // 0~100
};