#include <core/core_min.hpp>

#ifdef X_SYSTEM_LINUX

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using namespace std;

struct CPUStats {
	unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
};

static CPUStats getCPUStats() {
	ifstream statFile("/proc/stat");
	string   line;
	CPUStats stats = { 0, 0, 0, 0, 0, 0, 0, 0 };

	if (statFile.is_open()) {
		getline(statFile, line);
		istringstream ss(line);
		string        cpu;
		ss >> cpu >> stats.user >> stats.nice >> stats.system >> stats.idle >> stats.iowait >> stats.irq >> stats.softirq >> stats.steal;
		statFile.close();
	} else {
		cerr << "feiled to open stat file" << endl;
	}

	return stats;
}

double calculateCPUUsage(const CPUStats & prev, const CPUStats & curr) {
	unsigned long long prevIdle = prev.idle + prev.iowait;
	unsigned long long currIdle = curr.idle + curr.iowait;

	unsigned long long prevNonIdle = prev.user + prev.nice + prev.system + prev.irq + prev.softirq + prev.steal;
	unsigned long long currNonIdle = curr.user + curr.nice + curr.system + curr.irq + curr.softirq + curr.steal;

	unsigned long long prevTotal = prevIdle + prevNonIdle;
	unsigned long long currTotal = currIdle + currNonIdle;

	unsigned long long totalDiff = currTotal - prevTotal;
	unsigned long long idleDiff  = currIdle - prevIdle;

	return (double)(totalDiff - idleDiff) / totalDiff * 100.0;
}

int main() {
	while (true) {
		CPUStats prevStats = getCPUStats();
		this_thread::sleep_for(chrono::seconds(1));
		CPUStats currStats = getCPUStats();

		double cpuUsage = calculateCPUUsage(prevStats, currStats);
		cout << "CPU Usage: " << cpuUsage << "%" << endl;
	}

	return 0;
}

#elif defined(X_SYSTEM_DARWIN)
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <iostream>
#include <thread>
using namespace std;

#define CP_USER   0
#define CP_SYS    1
#define CP_IDLE   2
#define CP_NICE   3
#define CP_STATES 4

enum BYTE_UNITS { BYTES = 0, KILOBYTES = 1, MEGABYTES = 2, GIGABYTES = 3 };

template <class T>
static inline T convert_unit(T num, int to, int from = BYTES) {
	for (; from < to; from++) {
		num /= 1024;
	}
	return num;
}

static host_cpu_load_info_data_t _get_cpu_percentage() {
	kern_return_t             error;
	mach_msg_type_number_t    count;
	host_cpu_load_info_data_t r_load;
	mach_port_t               mach_port;

	count     = HOST_CPU_LOAD_INFO_COUNT;
	mach_port = mach_host_self();
	error     = host_statistics(mach_port, HOST_CPU_LOAD_INFO, (host_info_t)&r_load, &count);

	if (error != KERN_SUCCESS) {
		return host_cpu_load_info_data_t();
	}

	return r_load;
}

void getCpuUsePercentage() {
	host_cpu_load_info_data_t load1 = _get_cpu_percentage();

	std::this_thread::sleep_for(std::chrono::seconds(1));

	host_cpu_load_info_data_t load2 = _get_cpu_percentage();

	// pre load times
	unsigned long long current_user   = load1.cpu_ticks[CP_USER];
	unsigned long long current_system = load1.cpu_ticks[CP_SYS];
	unsigned long long current_nice   = load1.cpu_ticks[CP_NICE];
	unsigned long long current_idle   = load1.cpu_ticks[CP_IDLE];

	// Current load times
	unsigned long long next_user   = load2.cpu_ticks[CP_USER];
	unsigned long long next_system = load2.cpu_ticks[CP_SYS];
	unsigned long long next_nice   = load2.cpu_ticks[CP_NICE];
	unsigned long long next_idle   = load2.cpu_ticks[CP_IDLE];

	// Difference between the two
	unsigned long long diff_user   = next_user - current_user;
	unsigned long long diff_system = next_system - current_system;
	unsigned long long diff_nice   = next_nice - current_nice;
	unsigned long long diff_idle   = next_idle - current_idle;

	float value = static_cast<float>(diff_user + diff_system + diff_nice) / static_cast<float>(diff_user + diff_system + diff_nice + diff_idle) * 100.0;

	cout << value << endl;
}

int main() {
	while (1) {
		getCpuUsePercentage();
	}
	return 0;
}

#else

int main(int, char **) {
	xel::QuickExit();
	return 0;
}
#endif
