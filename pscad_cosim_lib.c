#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include "pscad_cosim.h"

// Maximum number of channels
#define MAX_CHANNELS 100
#define MAX_DATA_SIZE 100

// Global state variables
static int cosim_initialized = 0;
static EmtdcCosimulation_Channel* channels[MAX_CHANNELS];
static int num_channels = 0;

// Data structure for channel internal data
typedef struct {
    double* send_buffer;
    double* recv_buffer;
    double current_time;
    double last_send_time;
    int buffer_size;
} ChannelData;

// Forward declarations for channel methods
static double Channel_GetValue(EmtdcCosimulation_Channel* _this, double time, int index);
static void Channel_SetValue(EmtdcCosimulation_Channel* _this, double value, int index);
static void Channel_Send(EmtdcCosimulation_Channel* _this, double time);
static unsigned int Channel_GetChannelId(EmtdcCosimulation_Channel* _this);
static int Channel_GetSendSize(EmtdcCosimulation_Channel* _this);
static int Channel_GetRecvSize(EmtdcCosimulation_Channel* _this);

// Helper function to create a new channel
static EmtdcCosimulation_Channel* CreateChannel(unsigned int channel_id, int send_size, int recv_size) {
    EmtdcCosimulation_Channel* channel = malloc(sizeof(EmtdcCosimulation_Channel));
    if (!channel) {
        fprintf(stderr, "Error: Failed to allocate memory for channel %u\n", channel_id);
        return NULL;
    }
    
    // Initialize function pointers
    channel->GetValue = Channel_GetValue;
    channel->SetValue = Channel_SetValue;
    channel->Send = Channel_Send;
    channel->GetChannelId = Channel_GetChannelId;
    channel->GetSendSize = Channel_GetSendSize;
    channel->GetRecvSize = Channel_GetRecvSize;
    
    // Initialize channel properties
    channel->channel_id = channel_id;
    channel->send_size = send_size;
    channel->recv_size = recv_size;
    
    // Allocate and initialize private data
    ChannelData* data = malloc(sizeof(ChannelData));
    if (!data) {
        free(channel);
        fprintf(stderr, "Error: Failed to allocate memory for channel data %u\n", channel_id);
        return NULL;
    }
    
    data->send_buffer = calloc(send_size, sizeof(double));
    data->recv_buffer = calloc(recv_size, sizeof(double));
    data->current_time = 0.0;
    data->last_send_time = 0.0;
    data->buffer_size = (send_size > recv_size) ? send_size : recv_size;
    
    if (!data->send_buffer || !data->recv_buffer) {
        free(data->send_buffer);
        free(data->recv_buffer);
        free(data);
        free(channel);
        fprintf(stderr, "Error: Failed to allocate memory for channel buffers %u\n", channel_id);
        return NULL;
    }
    
    channel->private_data = data;
    
    printf("Created channel %u with send_size=%d, recv_size=%d\n", channel_id, send_size, recv_size);
    return channel;
}

// Channel method implementations
static double Channel_GetValue(EmtdcCosimulation_Channel* _this, double time, int index) {
    if (!_this || !_this->private_data) {
        fprintf(stderr, "Error: Invalid channel in GetValue\n");
        return 0.0;
    }
    
    ChannelData* data = (ChannelData*)_this->private_data;
    
    if (index < 0 || index >= _this->recv_size) {
        fprintf(stderr, "Error: Index %d out of range [0, %d) in GetValue\n", index, _this->recv_size);
        return 0.0;
    }
    
    if (time < data->current_time) {
        fprintf(stderr, "Warning: Time %f is less than previous time %f in GetValue\n", time, data->current_time);
    }
    
    data->current_time = time;
    
    // In a real implementation, this would read from the communication fabric
    // For this example, we'll simulate some data
    if (time > 0.0) {
        // Simulate receiving data from PSCAD
        data->recv_buffer[index] = sin(time * 2.0 * 3.14159 + index) * (index + 1);
    }
    
    printf("GetValue: Channel %u, time=%f, index=%d, value=%f\n", 
           _this->channel_id, time, index, data->recv_buffer[index]);
    
    return data->recv_buffer[index];
}

static void Channel_SetValue(EmtdcCosimulation_Channel* _this, double value, int index) {
    if (!_this || !_this->private_data) {
        fprintf(stderr, "Error: Invalid channel in SetValue\n");
        return;
    }
    
    if (index < 0 || index >= _this->send_size) {
        fprintf(stderr, "Error: Index %d out of range [0, %d) in SetValue\n", index, _this->send_size);
        return;
    }
    
    ChannelData* data = (ChannelData*)_this->private_data;
    data->send_buffer[index] = value;
    
    printf("SetValue: Channel %u, index=%d, value=%f\n", _this->channel_id, index, value);
}

static void Channel_Send(EmtdcCosimulation_Channel* _this, double time) {
    if (!_this || !_this->private_data) {
        fprintf(stderr, "Error: Invalid channel in Send\n");
        return;
    }
    
    ChannelData* data = (ChannelData*)_this->private_data;
    
    if (time <= data->last_send_time) {
        fprintf(stderr, "Warning: Send time %f is not greater than last send time %f\n", 
                time, data->last_send_time);
    }
    
    data->last_send_time = time;
    
    printf("Send: Channel %u, time=%f, values=[", _this->channel_id, time);
    for (int i = 0; i < _this->send_size; i++) {
        printf("%f", data->send_buffer[i]);
        if (i < _this->send_size - 1) printf(", ");
    }
    printf("]\n");
    
    // In a real implementation, this would send data through the communication fabric
    // For this example, we'll just print the data
}

static unsigned int Channel_GetChannelId(EmtdcCosimulation_Channel* _this) {
    if (!_this) {
        fprintf(stderr, "Error: Invalid channel in GetChannelId\n");
        return 0;
    }
    return _this->channel_id;
}

static int Channel_GetSendSize(EmtdcCosimulation_Channel* _this) {
    if (!_this) {
        fprintf(stderr, "Error: Invalid channel in GetSendSize\n");
        return 0;
    }
    return _this->send_size;
}

static int Channel_GetRecvSize(EmtdcCosimulation_Channel* _this) {
    if (!_this) {
        fprintf(stderr, "Error: Invalid channel in GetRecvSize\n");
        return 0;
    }
    return _this->recv_size;
}

// Global function implementations
void EmtdcCosimulation_InitializeCosimulation(
    const char* fabric_location,
    const char* hostname,
    int port,
    int client_id) {
    
    if (cosim_initialized) {
        fprintf(stderr, "Warning: Co-simulation already initialized\n");
        return;
    }
    
    printf("Initializing Co-Simulation:\n");
    printf("  Fabric Location: %s\n", fabric_location ? fabric_location : "NULL");
    printf("  Hostname: %s\n", hostname ? hostname : "NULL");
    printf("  Port: %d\n", port);
    printf("  Client ID: %d\n", client_id);
    
    // Validate parameters
    if (!fabric_location || !hostname) {
        fprintf(stderr, "Error: Invalid parameters for initialization\n");
        return;
    }
    
    if (client_id < 30000 || client_id > 65535) {
        fprintf(stderr, "Error: Client ID %d must be between 30000 and 65535\n", client_id);
        return;
    }
    
    // Initialize channel array
    for (int i = 0; i < MAX_CHANNELS; i++) {
        channels[i] = NULL;
    }
    
    // In a real implementation, this would:
    // 1. Load the Communication Fabric DLL
    // 2. Establish connection to PSCAD
    // 3. Configure channels based on the co-simulation setup
    
    // For this example, we'll create some default channels
    channels[0] = CreateChannel(10, 2, 2);  // Channel 10: send 2, receive 2
    channels[1] = CreateChannel(20, 1, 3);  // Channel 20: send 1, receive 3
    num_channels = 2;
    
    cosim_initialized = 1;
    printf("Co-simulation initialized successfully\n");
}

void EmtdcCosimulation_InitializeCosimulationFromFile(const char* file_name) {
    if (cosim_initialized) {
        fprintf(stderr, "Warning: Co-simulation already initialized\n");
        return;
    }
    
    if (!file_name) {
        fprintf(stderr, "Error: Invalid file name for initialization\n");
        return;
    }
    
    printf("Initializing Co-Simulation from file: %s\n", file_name);
    
    // In a real implementation, this would read the configuration file
    // and extract connection parameters
    
    // For this example, we'll use default parameters
    EmtdcCosimulation_InitializeCosimulation(
        "C:\\Program Files (x86)\\CommunicationFabric\\x86\\ComFab.dll",
        "localhost",
        34343,
        32000
    );
}

EmtdcCosimulation_Channel* EmtdcCosimulation_FindChannel(unsigned int channel_id) {
    if (!cosim_initialized) {
        fprintf(stderr, "Error: Co-simulation not initialized\n");
        return NULL;
    }
    
    for (int i = 0; i < num_channels; i++) {
        if (channels[i] && channels[i]->channel_id == channel_id) {
            printf("Found channel %u\n", channel_id);
            return channels[i];
        }
    }
    
    fprintf(stderr, "Error: Channel %u not found\n", channel_id);
    return NULL;
}

void EmtdcCosimulation_FinalizeCosimulation(void) {
    if (!cosim_initialized) {
        fprintf(stderr, "Warning: Co-simulation not initialized\n");
        return;
    }
    
    printf("Finalizing Co-Simulation...\n");
    
    // Clean up all channels
    for (int i = 0; i < num_channels; i++) {
        if (channels[i]) {
            ChannelData* data = (ChannelData*)channels[i]->private_data;
            if (data) {
                free(data->send_buffer);
                free(data->recv_buffer);
                free(data);
            }
            free(channels[i]);
            channels[i] = NULL;
        }
    }
    
    num_channels = 0;
    cosim_initialized = 0;
    
    printf("Co-simulation finalized\n");
}

int EmtdcCosimulation_IsInitialized(void) {
    return cosim_initialized;
}