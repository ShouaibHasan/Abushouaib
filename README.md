# PSCAD Co-Simulation C Interface

This project provides a complete C language implementation for connecting external applications with PSCAD/EMTDC using the Co-Simulation API. The implementation follows the official PSCAD Co-Simulation API documentation and provides a robust interface for bi-directional data exchange during power system simulations.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Examples](#examples)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)

## Overview

The PSCAD Co-Simulation API enables external applications to exchange data with PSCAD/EMTDC simulations in real-time. This C implementation provides:

- **EmtdcCosimulation_Channel Structure**: Complete implementation of the channel interface
- **Time Domain Data Exchange**: Robust handling of time-synchronized data
- **Multiple Channels**: Support for multiple communication channels
- **Error Handling**: Comprehensive error checking and validation
- **Memory Management**: Proper allocation and cleanup of resources

## Features

- ✅ Full implementation of PSCAD Co-Simulation API
- ✅ Support for multiple communication channels
- ✅ Time-domain data synchronization
- ✅ Configurable channel sizes (send/receive)
- ✅ Robust error handling and validation
- ✅ Memory leak prevention
- ✅ Cross-platform compatibility (Linux, Windows)
- ✅ Example applications and test cases
- ✅ Comprehensive documentation

## Requirements

### Build Requirements

- **C Compiler**: GCC 4.9+ or Clang 3.5+ or MSVC 2015+
- **Make**: GNU Make 3.81+ (optional, for Makefile builds)
- **Math Library**: Standard C math library (libm)

### Runtime Requirements

- **PSCAD/EMTDC**: Version 4.6+ with Co-Simulation Component
- **Communication Fabric**: ComFab.dll (provided with PSCAD installation)
- **Operating System**: Windows 7+, Linux (Ubuntu 16.04+, CentOS 7+)

### PSCAD Setup Requirements

1. Install PSCAD/EMTDC with Co-Simulation Component
2. Configure Co-Simulation Component in your PSCAD model
3. Set up appropriate channel IDs and data sizes
4. Generate configuration file (if using file-based initialization)

## Installation

### Quick Start

1. **Clone or Download** the source files:
   ```bash
   # If using git
   git clone <repository-url>
   cd pscad-cosim-c
   
   # Or download and extract the files
   ```

2. **Compile** using Make:
   ```bash
   make all
   ```

3. **Test** the demo application:
   ```bash
   make test
   ```

### Manual Compilation

If you prefer manual compilation:

```bash
# Basic compilation
gcc -Wall -Wextra -std=c99 -O2 pscad_cosim.c -lm -o pscad_cosim_demo

# Debug build
gcc -Wall -Wextra -std=c99 -g -DDEBUG pscad_cosim.c -lm -o pscad_cosim_demo_debug

# Library compilation (without main function)
gcc -Wall -Wextra -std=c99 -O2 -fPIC -DPSCAD_COSIM_LIBRARY -c pscad_cosim.c -o pscad_cosim.o
ar rcs libpscad_cosim.a pscad_cosim.o
```

### Cross-Platform Compilation

#### Windows (using MinGW)
```bash
make windows
# or manually:
x86_64-w64-mingw32-gcc -Wall -Wextra -std=c99 -O2 pscad_cosim.c -lm -o pscad_cosim_demo.exe
```

#### Advanced Build Options

```bash
# Using different compilers
make CC=clang                    # Use Clang
make CC=icc                      # Use Intel C Compiler
make CC=gcc-9                    # Use specific GCC version

# Custom optimization
make CFLAGS="-O3 -march=native"  # Maximum optimization
make CFLAGS="-Os"                # Size optimization

# Debug build with extra checks
make debug CFLAGS="-g -fsanitize=address -fsanitize=undefined"
```

## Usage

### Basic Usage Pattern

```c
#include "pscad_cosim.h"

int main() {
    // 1. Initialize co-simulation
    EmtdcCosimulation_InitializeCosimulation(
        "C:\\Program Files (x86)\\CommunicationFabric\\x86\\ComFab.dll",
        "localhost",
        34343,
        30001
    );
    
    // 2. Get communication channels
    EmtdcCosimulation_Channel* channel = EmtdcCosimulation_FindChannel(10);
    
    // 3. Simulation loop
    double time = 0.0;
    double time_step = 0.001;
    
    while (time < 1.0) {
        // Read from PSCAD (skip at time 0)
        if (time > 0) {
            double voltage = channel->GetValue(channel, time, 0);
            double current = channel->GetValue(channel, time, 1);
        }
        
        // Your algorithm here
        double control_signal = compute_control(voltage, current);
        
        // Send to PSCAD
        channel->SetValue(channel, control_signal, 0);
        channel->Send(channel, time + time_step);
        
        time += time_step;
    }
    
    // 4. Cleanup
    EmtdcCosimulation_FinalizeCosimulation();
    return 0;
}
```

### Configuration File Usage

```c
// Initialize using configuration file
EmtdcCosimulation_InitializeCosimulationFromFile("cosim_30001.cfg");
```

### Multiple Channels Example

```c
// Get multiple channels
EmtdcCosimulation_Channel* control_channel = EmtdcCosimulation_FindChannel(10);
EmtdcCosimulation_Channel* measurement_channel = EmtdcCosimulation_FindChannel(20);

// Verify channel properties
assert(control_channel->GetSendSize(control_channel) == 2);
assert(control_channel->GetRecvSize(control_channel) == 2);

// Use in simulation loop
for (double t = 0; t < end_time; t += dt) {
    if (t > 0) {
        // Read measurements
        double voltage = measurement_channel->GetValue(measurement_channel, t, 0);
        double power = measurement_channel->GetValue(measurement_channel, t, 1);
        
        // Compute control
        double control1 = pid_controller(voltage, voltage_ref);
        double control2 = power_controller(power, power_ref);
        
        // Send control signals
        control_channel->SetValue(control_channel, control1, 0);
        control_channel->SetValue(control_channel, control2, 1);
    }
    
    // Send for next time step
    control_channel->Send(control_channel, t + dt);
    measurement_channel->Send(measurement_channel, t + dt);
}
```

## API Reference

### Initialization Functions

#### `EmtdcCosimulation_InitializeCosimulation`
```c
void EmtdcCosimulation_InitializeCosimulation(
    const char* fabric_location,
    const char* hostname,
    int port,
    int client_id
);
```
- **Parameters**:
  - `fabric_location`: Path to ComFab.dll
  - `hostname`: PSCAD host IP/hostname
  - `port`: Communication port
  - `client_id`: Unique ID (30000-65535)
- **Description**: Initialize co-simulation with explicit parameters

#### `EmtdcCosimulation_InitializeCosimulationFromFile`
```c
void EmtdcCosimulation_InitializeCosimulationFromFile(const char* file_name);
```
- **Parameters**:
  - `file_name`: Path to configuration file
- **Description**: Initialize using configuration file

### Channel Management

#### `EmtdcCosimulation_FindChannel`
```c
EmtdcCosimulation_Channel* EmtdcCosimulation_FindChannel(unsigned int channel_id);
```
- **Parameters**:
  - `channel_id`: Channel identifier
- **Returns**: Pointer to channel or NULL if not found

### Channel Methods

#### `GetValue`
```c
double GetValue(EmtdcCosimulation_Channel* _this, double time, int index);
```
- **Parameters**:
  - `_this`: Channel pointer
  - `time`: Current simulation time
  - `index`: Data index (0 to RecvSize-1)
- **Returns**: Value received from PSCAD

#### `SetValue`
```c
void SetValue(EmtdcCosimulation_Channel* _this, double value, int index);
```
- **Parameters**:
  - `_this`: Channel pointer
  - `value`: Value to send
  - `index`: Data index (0 to SendSize-1)

#### `Send`
```c
void Send(EmtdcCosimulation_Channel* _this, double time);
```
- **Parameters**:
  - `_this`: Channel pointer
  - `time`: Valid time for sent values

### Cleanup

#### `EmtdcCosimulation_FinalizeCosimulation`
```c
void EmtdcCosimulation_FinalizeCosimulation(void);
```
- **Description**: Clean up resources and notify PSCAD

## Examples

### 1. Simple Voltage Controller

```c
#include "pscad_cosim.h"
#include <math.h>

double pid_controller(double error, double* integral, double* previous_error, double dt) {
    double kp = 1.0, ki = 0.1, kd = 0.01;
    
    *integral += error * dt;
    double derivative = (error - *previous_error) / dt;
    *previous_error = error;
    
    return kp * error + ki * (*integral) + kd * derivative;
}

int main() {
    EmtdcCosimulation_InitializeCosimulation(
        "C:\\Program Files (x86)\\CommunicationFabric\\x86\\ComFab.dll",
        "localhost", 34343, 30001
    );
    
    EmtdcCosimulation_Channel* channel = EmtdcCosimulation_FindChannel(10);
    if (!channel) return 1;
    
    double time = 0.0, dt = 0.001;
    double voltage_ref = 1.0;
    double integral = 0.0, previous_error = 0.0;
    
    while (time < 1.0) {
        double voltage = 0.0;
        if (time > 0) {
            voltage = channel->GetValue(channel, time, 0);
        }
        
        double error = voltage_ref - voltage;
        double control = pid_controller(error, &integral, &previous_error, dt);
        
        channel->SetValue(channel, control, 0);
        channel->Send(channel, time + dt);
        
        time += dt;
    }
    
    EmtdcCosimulation_FinalizeCosimulation();
    return 0;
}
```

### 2. Three-Phase Power System Control

```c
// Example for three-phase system with multiple channels
int main() {
    EmtdcCosimulation_InitializeCosimulationFromFile("power_system.cfg");
    
    EmtdcCosimulation_Channel* voltage_ch = EmtdcCosimulation_FindChannel(10);
    EmtdcCosimulation_Channel* current_ch = EmtdcCosimulation_FindChannel(20);
    EmtdcCosimulation_Channel* control_ch = EmtdcCosimulation_FindChannel(30);
    
    double time = 0.0, dt = 0.0001;  // 100µs time step
    
    while (time < 0.1) {  // 100ms simulation
        if (time > 0) {
            // Read three-phase voltages
            double va = voltage_ch->GetValue(voltage_ch, time, 0);
            double vb = voltage_ch->GetValue(voltage_ch, time, 1);
            double vc = voltage_ch->GetValue(voltage_ch, time, 2);
            
            // Read three-phase currents
            double ia = current_ch->GetValue(current_ch, time, 0);
            double ib = current_ch->GetValue(current_ch, time, 1);
            double ic = current_ch->GetValue(current_ch, time, 2);
            
            // Compute power and control
            double p = va*ia + vb*ib + vc*ic;  // Real power
            double control_signal = power_controller(p);
            
            // Send control signal
            control_ch->SetValue(control_ch, control_signal, 0);
        }
        
        // Send for next time step
        control_ch->Send(control_ch, time + dt);
        
        time += dt;
    }
    
    EmtdcCosimulation_FinalizeCosimulation();
    return 0;
}
```

## Configuration

### Configuration File Format

The configuration file uses INI format with the following sections:

```ini
[COSIMULATION]
fabric_location=C:\Program Files (x86)\CommunicationFabric\x86\ComFab.dll
hostname=localhost
port=34343
client_id=30001

[CHANNELS]
# Format: channel_id=send_size,recv_size,description
10=2,2,Voltage control channel
20=1,3,Power measurement channel
```

### Environment Variables

You can override default settings using environment variables:

```bash
export PSCAD_FABRIC_PATH="/path/to/ComFab.dll"
export PSCAD_HOSTNAME="192.168.1.100"
export PSCAD_PORT="34343"
export PSCAD_CLIENT_ID="30001"
```

## Troubleshooting

### Common Issues

1. **"Co-simulation not initialized" Error**
   - Ensure `EmtdcCosimulation_InitializeCosimulation` is called first
   - Check that PSCAD is running and listening on the specified port

2. **"Channel not found" Error**
   - Verify channel ID matches PSCAD Co-Simulation Component setup
   - Ensure PSCAD model includes the correct channel configuration

3. **"Communication Fabric DLL not found"**
   - Check ComFab.dll path in configuration
   - Ensure PSCAD installation includes Communication Fabric
   - On Linux, use Wine or native port if available

4. **Time Synchronization Issues**
   - Always send before receiving in the simulation loop
   - Never read values at time=0 (use initial conditions instead)
   - Ensure time values are monotonically increasing

5. **Memory Leaks**
   - Always call `EmtdcCosimulation_FinalizeCosimulation()` before exit
   - Use debug build to check for memory issues

### Debug Mode

Compile with debug flags for additional information:

```bash
make debug
./pscad_cosim_demo_debug
```

### Memory Checking

Use Valgrind to check for memory issues:

```bash
make memcheck
```

## Performance Considerations

1. **Time Step Size**: Smaller time steps provide better accuracy but increase computational load
2. **Channel Count**: More channels increase communication overhead
3. **Buffer Management**: The implementation uses efficient buffering for time-domain data
4. **Compiler Optimization**: Use `-O2` or `-O3` for production builds

## Contributing

Contributions are welcome! Please:

1. Follow the existing code style
2. Add tests for new features
3. Update documentation
4. Ensure cross-platform compatibility

## License

This implementation is provided as an example and reference. Please check PSCAD licensing terms for commercial use of the Co-Simulation API.

## References

- [PSCAD Co-Simulation API Documentation](https://www.pscad.com/webhelp-v5-ol/PSCAD/Parallel_and_High_Performance_Computing/Co-Simulation_API.htm)
- [PSCAD/EMTDC User Guide](https://www.pscad.com)
- [Communication Fabric Documentation](https://www.pscad.com/webhelp-v5-ol/PSCAD/Parallel_and_High_Performance_Computing/Communication_Fabric_(ComFab).htm)

---

**Note**: This is a reference implementation based on the PSCAD Co-Simulation API documentation. For production use, ensure proper testing with your specific PSCAD models and requirements. 
