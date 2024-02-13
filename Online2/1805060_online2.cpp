#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>

using namespace std;

#include "lcgrand.h" // Header file for Prime Modulus Multiplicative Linear Congruential Generator

int T, amount, bigs, I, inv_level, next_event_type, num_events,
    N, D, smalls;
float area_holding, area_shortage, holding_cost, incremental_cost, maxlag,
      beta_D, minlag, prob_distrib_demand[26], setup_cost,
      shortage_cost, sim_time, time_last_event, time_next_event[5],
      total_ordering_cost;
float E_K, E_i, E_m, E_M;
float express_setup_cost, express_incremental_cost, express_minlag, express_maxlag;
float Avg_before_express, Avg_after_express, Expected_express_count;

int total_express_orders;
float total_costs_no_express, total_costs_with_express;

std::ifstream infile;
std::ofstream outfile;

void initialize(void);
void timing(void);
void order_arrival(void);
void demand(void);
void evaluate(void);
void report(void);
void update_time_avg_stats(void);
float expon(float mean);
int random_integer(float prob_distrib []);
float uniform(float a, float b);

void initialize(void) /* Initialization function. */
{
    /* Initialize the simulation clock. */
    sim_time = 0.0;
    /* Initialize the state variables. */
    inv_level = I;
    time_last_event = 0.0;
    /* Initialize the statistical counters. */
    total_ordering_cost = 0.0;
    area_holding = 0.0;
    area_shortage = 0.0;
    // express order
    express_setup_cost = E_K;
    express_incremental_cost = E_i;
    express_minlag = E_m;
    express_maxlag = E_M;

    /* Initialize the event list. Since no order is outstanding, the order-
    arrival event is eliminated from consideration. */
    time_next_event[1] = 1.0e+30;
    time_next_event[2] = sim_time + expon(beta_D);
    time_next_event[3] = N;
    time_next_event[4] = 0.0;
}

void timing(void) /* Timing function. */
{
    int i;
    float min_time_next_event = 1.0e+29;
    next_event_type = 0;
    /* Determine the event type of the next event to occur. */
    for (i = 1; i <= num_events; ++i)
        if (time_next_event[i] < min_time_next_event)
        {
            min_time_next_event = time_next_event[i];
            next_event_type = i;
        }
    /* Check to see whether the event list is empty. */
    if (next_event_type == 0)
    {
        /* The event list is empty, so stop the simulation. */
        std::cerr << "Event list empty at time " << sim_time << std::endl;
        std::exit(1);
    }
    /* The event list is not empty, so advance the simulation clock. */
    sim_time = min_time_next_event;
}

void order_arrival(void) /* Order arrival event function. */
{
    /* Increment the inventory level by the amount ordered. */
    inv_level += amount;
    /* Since no order is now outstanding, eliminate the order-arrival event from
    consideration. */
    time_next_event[1] = 1.0e+30;
}

void demand(void) /* Demand event function. */
{
    /* Decrement the inventory level by a generated demand size. */
    inv_level -= random_integer(prob_distrib_demand);
    /* Schedule the time of the next demand. */
    time_next_event[2] = sim_time + expon(beta_D);
}

void evaluate(void) /* Inventory-evaluation event function. */
{
    if (inv_level < 0) {
        // The inventory level is less than zero, so place an express order.
        amount = -inv_level;
        total_costs_with_express += express_setup_cost + express_incremental_cost * amount;
        time_next_event[1] = sim_time + uniform(express_minlag, express_maxlag);
        total_express_orders += 1;
    } else if (inv_level < smalls)
    {
        /* The inventory level is less than smalls, so place an order for the
        appropriate amount. */
        amount = bigs - inv_level;
        total_ordering_cost += setup_cost + incremental_cost * amount;
        /* Schedule the arrival of the order. */
        time_next_event[1] = sim_time + uniform(minlag, maxlag);
    }
    /* Regardless of the place-order decision, schedule the next inventory
    evaluation. */
    time_next_event[4] = sim_time + 1.0;
}

void report(void) /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */
    float avg_holding_cost, avg_ordering_cost, avg_shortage_cost;
    float avg_express_orders = total_express_orders / N;
    avg_ordering_cost = total_ordering_cost / N;
    avg_holding_cost = holding_cost * area_holding / N;
    avg_shortage_cost = shortage_cost * area_shortage / N;
    float avg_total_costs_no_express = total_ordering_cost / T;
    float avg_total_costs_with_express = total_costs_with_express / T;
    outfile << "\n\n(" << smalls << "," << std::setw(3) << std::setfill(' ') << bigs << ")" << "            " << std::setw(8) << std::setfill(' ') << avg_total_costs_no_express
            << "            " << std::setw(8) << std::setfill(' ') << avg_total_costs_with_express << "            " << std::setw(8) << std::setfill(' ') << avg_express_orders; //  << "            " << std::setw(8) << std::setfill(' ') << avg_shortage_cost;
}



void update_time_avg_stats(void) /* Update area accumulators for time-average statistics. */
{
    float time_since_last_event;
    /* Compute time since last event, and update last-event-time marker. */
    time_since_last_event = sim_time - time_last_event;
    time_last_event = sim_time;
    /* Determine the status of the inventory level during the previous interval.
    If the inventory level during the previous interval was negative, update
    area_shortage. If it was positive, update area_holding. If it was zero,
    no update is needed. */
    if (inv_level < 0)
        area_shortage -= inv_level * time_since_last_event;
    else if (inv_level > 0)
        area_holding += inv_level * time_since_last_event;
}

int random_integer(float prob_distrib[]) /* Random integer generationfunction. */
{
    int i;
    float u;
    /* Generate a U(0,1) random variate. */
    u = lcgrand(1);
    /* Return a random integer in accordance with the (cumulative) distribution
    function prob_distrib. */
    for (i = 1; u >= prob_distrib[i]; ++i)
        ;
    return i;
}

float uniform(float a, float b) /* Uniform variate generation function. */
{
    /* Return a U(a,b) random variate. */
    return a + lcgrand(1) * (b - a);
}

float expon(float mean)
{
    return -mean * log(lcgrand(1));
}



int main() /* Main function. */
{
    int i, P;
    /* Open input and output files. */
    infile.open("in.txt", std::ios::in);
    outfile.open("out.txt", std::ios::out);
    /* Specify the number of events for the timing function. */
    num_events = 4;
    /* Read input parameters. */
    infile >> T >> I >> N >> P >> D
           >> beta_D >> setup_cost >> incremental_cost >> holding_cost
           >> shortage_cost >> E_K >> E_i >> minlag >> maxlag >> E_m >> E_M;
    for (i = 1; i <= D; ++i)
        infile >> prob_distrib_demand[i];
    /* Write report heading and input parameters. */
    outfile << "------Single-Product Inventory System------\n\n";
    outfile << "Initial inventory level: " << I << " items\n\n";
    outfile << "Number of demand sizes: "  << D << "\n\n";
    outfile << "Distribution function of demand sizes: ";
    for (i = 1; i <= D; ++i)
        outfile << std::fixed << std::setprecision(2) << prob_distrib_demand[i] << " ";

    outfile << "\n\nMean inter-demand time: " << beta_D << " months\n\n";
    outfile << "Delivery lag range: "<< minlag << " to " << maxlag << " months\n\n";
    outfile << "Express Order lag range: "<< E_m << " to " << E_M << " months\n\n";
    outfile << "Length of simulation: " << N << " months\n\n";
    outfile << "Costs:\n"
            << "K = " << setup_cost << "\n"
            << "i = " << incremental_cost << "\n"
            << "h = " << holding_cost << "\n"
            << "pi = " << shortage_cost << "\n"
            << "E_K = " << E_K << "\n"
            << "E_i = " << E_i << "\n\n";
    outfile << "Number of policies: " << P << "\n";
    outfile << "Number of Trials: " << T << "\n\n";
    outfile << "Policies:\n";
    outfile << "--------------------------------------------------------------------------------------------------\n";
    outfile << " Policy       Avg_before_express     Avg_after_express   Expected_express_count\n";
    outfile << "--------------------------------------------------------------------------------------------------";

    /* Run the simulation varying the inventory policy. */
    for (i = 1; i <= P; ++i)
    {
        /* Read the inventory policy, and initialize the simulation. */
        infile >> smalls >> bigs;
        initialize();
        
        /* Run the simulation until it terminates after an end-simulation event
        (type 3) occurs. */
        for (int t = 1; t <= T; ++t)
        {
            do
            {
                /* Determine the next event. */
                timing();
                /* Update time-average statistical accumulators. */
                update_time_avg_stats();
                /* Invoke the appropriate event function. */
                switch (next_event_type)
                {
                case 1:
                    order_arrival();
                    break;
                case 2:
                    demand();
                    break;
                case 4:
                    evaluate();
                    break;
                case 3:
                    report();
                    break;
                }
                /* If the event just executed was not the end-simulation event (type 3),
                continue simulating. Otherwise, end the simulation for the current
                (s,S) pair and go on to the next pair (if any). */
            } while (next_event_type != 3);
        }

    }

    /* End the simulations. */
    infile.close();
    outfile.close();
    return 0;
}