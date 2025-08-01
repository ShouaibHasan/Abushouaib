#ifndef PSCAD_COSIM_H
#define PSCAD_COSIM_H

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration of the channel structure
typedef struct EmtdcCosimulation_Channel EmtdcCosimulation_Channel;

// Structure definition for EmtdcCosimulation_Channel
struct EmtdcCosimulation_Channel {
    // Function pointers for channel methods
    double (*GetValue)(EmtdcCosimulation_Channel* _this, double time, int index);
    void (*SetValue)(EmtdcCosimulation_Channel* _this, double value, int index);
    void (*Send)(EmtdcCosimulation_Channel* _this, double time);
    unsigned int (*GetChannelId)(EmtdcCosimulation_Channel* _this);
    int (*GetSendSize)(EmtdcCosimulation_Channel* _this);
    int (*GetRecvSize)(EmtdcCosimulation_Channel* _this);
    
    // Private data members (implementation specific)
    void* private_data;
    unsigned int channel_id;
    int send_size;
    int recv_size;
};

// Global initialization functions
void EmtdcCosimulation_InitializeCosimulation(
    const char* fabric_location,
    const char* hostname,
    int port,
    int client_id
);

// Alternative initialization with config file
void EmtdcCosimulation_InitializeCosimulationFromFile(
    const char* file_name
);

// Channel management functions
EmtdcCosimulation_Channel* EmtdcCosimulation_FindChannel(
    unsigned int channel_id
);

// Cleanup function
void EmtdcCosimulation_FinalizeCosimulation(void);

// Helper function to check if co-simulation is initialized
int EmtdcCosimulation_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif // PSCAD_COSIM_H