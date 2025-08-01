#include <stdio.h>

// Forward declarations for PSCAD co-simulation functions
// These should match the actual PSCAD API - you may need to adjust based on your PSCAD installation

// Structure for the co-simulation channel
typedef struct EmtdcCosimulation_Channel EmtdcCosimulation_Channel;

struct EmtdcCosimulation_Channel {
    double (*GetValue)(EmtdcCosimulation_Channel* _this, double time, int index);
    void (*SetValue)(EmtdcCosimulation_Channel* _this, double value, int index);
    void (*Send)(EmtdcCosimulation_Channel* _this, double time);
    unsigned int (*GetChannelId)(EmtdcCosimulation_Channel* _this);
    int (*GetSendSize)(EmtdcCosimulation_Channel* _this);
    int (*GetRecvSize)(EmtdcCosimulation_Channel* _this);
    void* private_data;
    unsigned int channel_id;
    int send_size;
    int recv_size;
};

// Function declarations - these should be provided by PSCAD's co-simulation library
void EmtdcCosimulation_InitializeCosimulation(
    const char* fabric_location,
    const char* hostname,
    int port,
    int client_id
);

EmtdcCosimulation_Channel* EmtdcCosimulation_FindChannel(unsigned int channel_id);
void EmtdcCosimulation_FinalizeCosimulation(void);

// --- Global variable to hold the communication channel ---
const int channel_id = 10; // From your PSCAD component screenshot
EmtdcCosimulation_Channel* channel = NULL;  // Initialize to NULL, assign in function
static int is_initialized = 0;  // Track initialization state

/**
 * @brief Initializes the connection to the PSCAD co-simulation.
 * Call this function once at the beginning of the simulation.
 * @return 0 on failure, 1 on success.
 */
int InitializeCoSimulation() {
    if (is_initialized) {
        return 1;  // Already initialized
    }
    
    printf("--- Initializing Co-simulation Library ---\n");

    const char* fabric_location = "C:\\Program Files (x86)\\CommunicationFabric\\x86\\ComFab.dll";
    const char* host_name = "localhost"; // Assuming PSCAD is running on the same machine
    const int port = 34343; // Default PSCAD co-simulation port
    const int client_id = 40001; // From your PSCAD component screenshot

    // Initialize the co-simulation
    EmtdcCosimulation_InitializeCosimulation(fabric_location, host_name, port, client_id);

    // Find the channel
    channel = EmtdcCosimulation_FindChannel(channel_id);

    if (channel == NULL) {
        printf("ERROR: Could not find channel with ID: %d\n", channel_id);
        EmtdcCosimulation_FinalizeCosimulation();
        return 0;
    }
    
    printf("Successfully found channel with ID: %d\n", channel_id);
    printf("Channel send size: %d\n", channel->GetSendSize(channel));
    printf("Channel recv size: %d\n", channel->GetRecvSize(channel));
    
    is_initialized = 1;
    return 1;
}

/**
 * @brief Executes one time step of the co-simulation logic.
 * Call this function inside the PSCAD simulation loop at each time step.
 * @param time The current simulation time from PSCAD.
 * @param time_step The simulation time step.
 */
void ExecuteTimeStep(double time, double time_step) {
    // Initialize if not already done
    if (!is_initialized) {
        if (!InitializeCoSimulation()) {
            return; // Failed to initialize
        }
    }
    
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
    // Example: Simple doubling operation - replace with your algorithm
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
    if (is_initialized && channel != NULL) {
        printf("\nSimulation loop finished. Finalizing connection.\n");
        EmtdcCosimulation_FinalizeCosimulation();
        channel = NULL;
        is_initialized = 0;
    }
}

/**
 * @brief Alternative function for PSCAD integration
 * This can be called directly from PSCAD's simulation loop
 * @param current_time Current simulation time
 * @param time_step Simulation time step
 * @param input_value Value received from PSCAD
 * @return Computed output value to send back to PSCAD
 */
double ProcessCoSimulationStep(double current_time, double time_step, double input_value) {
    // Initialize if not already done
    if (!is_initialized) {
        if (!InitializeCoSimulation()) {
            return 0.0; // Return default value if initialization fails
        }
    }
    
    if (channel == NULL) {
        return 0.0; // Return default value if channel not available
    }

    // Your custom algorithm here
    // Example: Simple proportional control
    double output_value = input_value * 2.0;
    
    // Send the computed value back to PSCAD
    channel->SetValue(channel, output_value, 0);
    channel->Send(channel, current_time + time_step);
    
    return output_value;
}