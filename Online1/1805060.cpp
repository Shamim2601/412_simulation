#include <iostream>
#include <fstream>
#include <queue>
#include <limits>
#include <cmath>
#include "lcgrand.h"

using namespace std;

class MultipleServerQueueSimulator {
private:
    // Constants
    static const int BUSY = 1;
    static const int IDLE = 0;

    // Simulation parameters and statistics
    float A, S;
    int s, N;
    int event_count, arrive_cust_count, depart_cust_count;
    float sim_time, time_last_event, next_arrival_time, next_departure_time;
    int num_custs_delayed, num_in_q, server_status, next_event_type, num_events;
    float total_of_delays, area_num_in_q, area_server_status;

    // File names
    const char* input_file_name;
    const char* output_file_name;
    const char* output_file_name2;

    // Queue for arrival times
    queue<float> arrival_times_q;

    // Output file stream
    ofstream outfile;
    ofstream outfile2;

public:
    // Constructor
    MultipleServerQueueSimulator(const char* in_filename, const char* out_filename, const char* out_filename2)
        : input_file_name(in_filename), output_file_name(out_filename), output_file_name2(out_filename2) {}

    // Function prototypes
    void initialize();
    float expon(float mean);
    void timing();
    void update_time_avg_stats();
    void arrive();
    void depart();
    void report();
    void simulate();
};

// Function definitions

void MultipleServerQueueSimulator::initialize() {
    sim_time = 0.0;
    server_status = IDLE;
    num_in_q = 0;
    event_count = 0;
    arrive_cust_count = 0;
    depart_cust_count = 0;
    time_last_event = 0.0;
    num_custs_delayed = 0;
    total_of_delays = 0.0;
    area_num_in_q = 0.0;
    area_server_status = 0.0;

    next_arrival_time = sim_time + expon(A);
    next_departure_time = numeric_limits<float>::infinity();
}

float MultipleServerQueueSimulator::expon(float mean) {
    return -mean * log(lcgrand(1));
}

void MultipleServerQueueSimulator::timing() {
    event_count++;
    // Determine the event type of the next event to occur
    if (next_arrival_time < next_departure_time) {
        next_event_type = 1;
        outfile2<<event_count<<". Next event: Customer "<<++arrive_cust_count<<" Arrival"<<endl;
        sim_time = next_arrival_time;
    } else {
        next_event_type = 2;
        outfile2<<event_count<<". Next event: Customer "<<++depart_cust_count<<" Departure"<<endl;
        sim_time = next_departure_time;
    }
}

void MultipleServerQueueSimulator::update_time_avg_stats() {
    float time_since_last_event = sim_time - time_last_event;
    time_last_event = sim_time;
    area_num_in_q += num_in_q * time_since_last_event;
    area_server_status += server_status * time_since_last_event;
}

void MultipleServerQueueSimulator::arrive() {
    float delay;
    next_arrival_time = sim_time + expon(A);
    if (server_status >= s) {
        num_in_q++;
        arrival_times_q.push(sim_time);
    } else {
        delay = 0.0;
        server_status++;
        total_of_delays += delay;
        num_custs_delayed++;
        outfile2<<endl<<"---------No. of customers delayed: "<<num_custs_delayed<<"--------"<<endl<<endl;

        // server_status = BUSY;
        next_departure_time = sim_time + expon(S);
    }
    
}

void MultipleServerQueueSimulator::depart() {
    float delay;
    if (num_in_q == 0) {
        // server_status = IDLE;
        if(server_status) server_status--;
        next_departure_time = numeric_limits<float>::infinity();
    } else {
        num_in_q--;
        delay = sim_time - arrival_times_q.front();
        arrival_times_q.pop();
        total_of_delays += delay;
        num_custs_delayed++;
        outfile2<<endl<<"---------No. of customers delayed: "<<num_custs_delayed<<"--------"<<endl<<endl;


        next_departure_time = sim_time + expon(S);
    }


}

void MultipleServerQueueSimulator::report() {
    outfile << "Avg delay in queue: " << total_of_delays / num_custs_delayed << " minutes" << endl;
    outfile << "Avg number in queue: " << area_num_in_q / sim_time << endl;
    outfile << "Server utilization: " << area_server_status / sim_time << endl;
    outfile << "Time simulation ended: " << sim_time << " minutes" << endl;
}

void MultipleServerQueueSimulator::simulate() {
    ifstream infile;
    infile.open(input_file_name);
    if (!infile.is_open()) {
        cerr << "Error opening input file: " << input_file_name << endl;
        exit(1);
    }
    infile >> s >> A >> S >> N; // the mean inter-arrival time, the mean service time and the
                           // total number of delays required

    outfile.open(output_file_name);
    if (!outfile.is_open()) {
        cerr << "Error opening output file: " << output_file_name << endl;
        exit(1);
    }

    outfile2.open(output_file_name2);
    if (!outfile2.is_open()) {
        cerr << "Error opening output file 2: " << output_file_name2 << endl;
        exit(1);
    }

    num_events = 2;


    // Initialize the simulation
    initialize();

    // Run the simulation while more delays are still needed
    while (num_custs_delayed < N) {
        // Determine the next event
        timing();

        // Update time-average statistical accumulators
        update_time_avg_stats();

        // Invoke event function
        switch (next_event_type) {
            case 1:
                arrive();

                break;
            case 2:
                depart();

                break;
        }


    }


    // Report heading and input parameters
    outfile << "----Multi-Server Queueing System----" << endl << endl;
    outfile << "Number of servers: " << s << endl;
    outfile << "Mean inter-arrival time: " << A << " minutes" << endl;
    outfile << "Mean service time: " << S << " minutes" << endl;
    outfile << "Number of customers: " << N << endl << endl;

    // Report results
    outfile << "Avg delay in queue: " << total_of_delays / num_custs_delayed << " minutes" << endl;
    outfile << "Avg number in queue: " << area_num_in_q / sim_time << endl;
    outfile << "Server utilization: " << area_server_status / sim_time << endl;
    outfile << "Time simulation ended: " << sim_time << " minutes" << endl;


    // Close files
    outfile.close();
    outfile2.close();
    infile.close();
}

// Main function
int main() {
    MultipleServerQueueSimulator simulator("in.txt", "results.txt", "event_orders.txt");
    simulator.simulate();
    return 0;
}