#include <stdio.h>

// For Sleep/sleep functions
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Add the necessary PSCAD co-simulation header
// Note: You may need to adjust this path based on your PSCAD installation
#include "EmtdcCosimulation.h"

// Function declarations for PSCAD co-simulation functions
// These should normally be in a header file, but adding here for compilation
typedef struct EmtdcCosimulation_Channel EmtdcCosimulation_Channel;

struct EmtdcCosimulation_Channel {
    double (*GetValue)(EmtdcCosimulation_Channel* channel, double time, int index);
    void (*SetValue)(EmtdcCosimulation_Channel* channel, double value, int index);
    void (*Send)(EmtdcCosimulation_Channel* channel, double time);
};

// Function declarations
EmtdcCosimulation_Channel* EmtdcCosimulation_FindChannel(int channel_id);
void EmdtcCosimulation_FinalizeCoSimulation(void);

// --- Global variable to hold the communication channel ---
const int channel_id = 10; // From your PSCAD component screenshot
EmtdcCosimulation_Channel* channel = NULL; // Initialize to NULL, assign in function

/**
 * @brief Initializes the connection to the PSCAD co-simulation.
 * Call this function once at the beginning of the simulation.
 * @return 0 on failure, 1 on success.
 */
int InitializeCoSimulation() {
    printf("--- Initializing Co-simulation Library ---\n");

    const char* host_name = "localhost"; // Assuming PSCAD is running on the same machine
    const int port = 5000; // Example port, may need to be configured in PSCAD
    const int client_id = 40001; // From your PSCAD component screenshot
    const char* client_name = "C_CoSim_Client";
    const int timeout = 10000; // 10 second timeout

    channel = EmtdcCosimulation_FindChannel(channel_id); // Fixed function name

    if (channel == NULL) {
        printf("ERROR: Could not find channel with ID: %d\n", channel_id);
        EmdtcCosimulation_FinalizeCoSimulation();
        return 0;
    }
    printf("Successfully found channel with ID: %d\n", channel_id);
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
        return; // Do nothing if not initialized
    }

    // --- Receive Data from PSCAD ---
    double val_from_pscad = 0.0;
    if (time > 0) {
        val_from_pscad = channel->GetValue(channel, time, 0);
    }

    // --- Perform Computations ---
    double val_to_pscad = val_from_pscad * 2.0;

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
        EmdtcCosimulation_FinalizeCoSimulation();
        channel = NULL;
    }
}