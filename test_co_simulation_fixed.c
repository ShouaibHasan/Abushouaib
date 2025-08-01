#define _DEFAULT_SOURCE  // For usleep on Linux
#include <stdio.h>
#include "pscad_cosim.h"  // Include the PSCAD co-simulation header

// For Sleep/sleep functions
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// --- Global variable to hold the communication channel ---
const int channel_id = 10; // From your PSCAD component screenshot
EmtdcCosimulation_Channel* channel = NULL;  // Initialize to NULL, assign in function

/**
 * @brief Initializes the connection to the PSCAD co-simulation.
 * Call this function once at the beginning of the simulation.
 * @return 0 on failure, 1 on success.
 */
int InitializeCoSimulation() {
    printf("--- Initializing Co-simulation Library ---\n");

    const char* fabric_location = "C:\\Program Files (x86)\\CommunicationFabric\\x86\\ComFab.dll";
    const char* host_name = "localhost"; // Assuming PSCAD is running on the same machine
    const int port = 34343; // Default PSCAD co-simulation port
    const int client_id = 40001; // From your PSCAD component screenshot

    // Initialize the co-simulation using the correct function name
    EmtdcCosimulation_InitializeCosimulation(fabric_location, host_name, port, client_id);

    // Find the channel using the correct function name
    channel = EmtdcCosimulation_FindChannel(channel_id);

    if (channel == NULL) {
        printf("ERROR: Could not find channel with ID: %d\n", channel_id);
        EmtdcCosimulation_FinalizeCosimulation();  // Correct function name
        return 0;
    }
    
    printf("Successfully found channel with ID: %d\n", channel_id);
    printf("Channel send size: %d\n", channel->GetSendSize(channel));
    printf("Channel recv size: %d\n", channel->GetRecvSize(channel));
    
    return 1;
}

/**
 * @brief Executes one time step of the co-simulation logic.
 * Call this function inside the PSCAD simulation loop at each time step.
 * @param time The current simulation time from PSCAD.
 * @param time_step The simulation time step.
 */
void ExecuteTimeStep(double time, double time_step) {
    if (channel == NULL) {
        printf("WARNING: Channel not initialized, skipping time step\n");
        return; // Do nothing if not initialized
    }

    // --- Receive Data from PSCAD ---
    double val_from_pscad = 0.0;
    if (time > 0) {
        val_from_pscad = channel->GetValue(channel, time, 0);
        printf("Received from PSCAD at time %f: %f\n", time, val_from_pscad);
    }

    // --- Perform Computations ---
    double val_to_pscad = val_from_pscad * 2.0;
    printf("Sending to PSCAD at time %f: %f\n", time, val_to_pscad);

    // --- Send Data to PSCAD ---
    channel->SetValue(channel, val_to_pscad, 0);
    channel->Send(channel, time + time_step);
}

/**
 * @brief Finalizes the connection to the PSCAD co-simulation.
 * Call this function once at the end of the simulation.
 */
void FinalizeCoSimulation() {
    if (channel != NULL) {
        printf("\nSimulation loop finished. Finalizing connection.\n");
        EmtdcCosimulation_FinalizeCosimulation();  // Correct function name
        channel = NULL;
    }
}

/**
 * @brief Example main function demonstrating the co-simulation workflow
 */
int main() {
    printf("=== PSCAD Co-Simulation Test ===\n");
    
    // Initialize co-simulation
    if (!InitializeCoSimulation()) {
        printf("Failed to initialize co-simulation. Exiting.\n");
        return 1;
    }
    
    // Example simulation loop
    double time = 0.0;
    double time_step = 0.001;  // 1ms time step
    double end_time = 0.01;    // 10ms simulation
    
    printf("\nStarting simulation loop...\n");
    
    while (time < end_time) {
        printf("\n--- Time Step: %f ---\n", time);
        ExecuteTimeStep(time, time_step);
        time += time_step;
        
        // Small delay to make output readable
        #ifdef _WIN32
        Sleep(100);  // 100ms delay
        #else
        usleep(100000);  // 100ms delay
        #endif
    }
    
    // Finalize co-simulation
    FinalizeCoSimulation();
    
    printf("\nCo-simulation test completed successfully.\n");
    return 0;
}