# PSCAD Co-Simulation Error Fixes Summary

## Original Compilation Errors

The original code had several compilation errors when compiled with Microsoft Visual Studio:

```
C2143: syntax error: missing '{' before '*'
C2099: initializer is not a constant
C2223: left of '->GetValue' must point to struct/union
C2223: left of '->SetValue' must point to struct/union
C2223: left of '->Send' must point to struct/union
```

## Root Causes and Fixes

### 1. **Missing Header File**
**Error**: `C2143: syntax error: missing '{' before '*'`
**Cause**: Missing include for the PSCAD co-simulation header file
**Fix**: Added `#include "pscad_cosim.h"` at the top of the file

### 2. **Incorrect Function Names (Typos)**
**Error**: Function names had typos
**Original**: `EmdtcCosimulation_FindChannel`, `EmdtcCosimulation_FinalizeCoSimulation`
**Fixed**: `EmtdcCosimulation_FindChannel`, `EmtdcCosimulation_FinalizeCosimulation`

### 3. **Global Variable Initialization**
**Error**: `C2099: initializer is not a constant`
**Cause**: Trying to initialize global pointer with function call
```c
// WRONG:
EmtdcCosimulation_Channel * channel = EmtdcCosimulation_FindChannel(10);
```
**Fix**: Initialize to NULL and assign in function
```c
// CORRECT:
EmtdcCosimulation_Channel* channel = NULL;
// Then in function:
channel = EmtdcCosimulation_FindChannel(channel_id);
```

### 4. **Missing Initialization Call**
**Cause**: Channel was being used without proper co-simulation initialization
**Fix**: Added proper initialization sequence:
```c
EmtdcCosimulation_InitializeCosimulation(fabric_location, host_name, port, client_id);
channel = EmtdcCosimulation_FindChannel(channel_id);
```

### 5. **Platform-Specific Issues**
**Issue**: `usleep` function not available on all systems
**Fix**: Added proper feature test macro:
```c
#define _DEFAULT_SOURCE  // For usleep on Linux
```

## Complete Fixed Code

### For Standalone Testing (`test_co_simulation_fixed.c`):
```c
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
const int channel_id = 10;
EmtdcCosimulation_Channel* channel = NULL;  // Initialize to NULL

int InitializeCoSimulation() {
    printf("--- Initializing Co-simulation Library ---\n");

    const char* fabric_location = "C:\\Program Files (x86)\\CommunicationFabric\\x86\\ComFab.dll";
    const char* host_name = "localhost";
    const int port = 34343; // Default PSCAD co-simulation port
    const int client_id = 40001;

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
    return 1;
}

void ExecuteTimeStep(double time, double time_step) {
    if (channel == NULL) {
        return;
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

void FinalizeCoSimulation() {
    if (channel != NULL) {
        printf("\nSimulation loop finished. Finalizing connection.\n");
        EmtdcCosimulation_FinalizeCosimulation();  // Correct function name
        channel = NULL;
    }
}
```

### For PSCAD Integration (`pscad_integration.c`):
This version includes the structure definitions and can be compiled directly with PSCAD without needing external header files.

## Configuration for PSCAD

### 1. **Channel Configuration**
- Channel ID: 10 (as specified in your PSCAD component)
- Port: 34343 (default PSCAD co-simulation port)
- Client ID: 40001 (as shown in your screenshot)

### 2. **Required Files for PSCAD Project**
- `pscad_integration.c` - Main integration code
- ComFab.dll - Communication Fabric library (from PSCAD installation)

### 3. **Compilation in PSCAD**
When PSCAD compiles your project, it should now find all the necessary definitions and functions.

## Key Points for Success

1. **Always initialize before using**: Call `InitializeCoSimulation()` before any channel operations
2. **Check for NULL pointers**: Always verify channel is not NULL before using
3. **Proper time handling**: Don't read values at time=0 to avoid deadlock
4. **Clean shutdown**: Always call `FinalizeCoSimulation()` when done

## Common PSCAD Integration Issues

### Issue 1: ComFab.dll Not Found
**Solution**: Ensure the path to ComFab.dll is correct for your PSCAD installation:
```c
const char* fabric_location = "C:\\Program Files (x86)\\CommunicationFabric\\x86\\ComFab.dll";
```

### Issue 2: Channel Not Found
**Solution**: Verify the channel ID matches what's configured in your PSCAD Co-Simulation Component

### Issue 3: Port Connection Issues
**Solution**: Ensure PSCAD is listening on the correct port (usually 34343)

## Testing Steps

1. Compile the fixed code without errors
2. Run PSCAD with Co-Simulation Component configured
3. Execute your external application
4. Verify data exchange is working

The fixed code should now compile successfully with both GCC and Microsoft Visual Studio compilers and integrate properly with PSCAD co-simulation.