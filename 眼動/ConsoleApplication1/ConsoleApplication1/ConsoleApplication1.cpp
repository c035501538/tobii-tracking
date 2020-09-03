#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>
#include <stdio.h>
#include "SerialPort.h"
#include <assert.h>
#include <string>
#include <iostream>
#include <typeinfo>
#include <windows.h>
#include <tchar.h>
#include "SerialClass.h"
#include<stdlib.h>


#pragma warning( disable : 4996 )
using namespace std;

float x, y;
string xy,command;

char output[MAX_DATA_LENGTH];
char incomingData[MAX_DATA_LENGTH];

// change the name of the port with the port name of your computer
// must remember that the backslashes are essential so do not remove them
char port[] = "\\\\.\\COM4";

void gaze_point_callback(tobii_gaze_point_t const* gaze_point, void* /* user_data */)
{
    // Check that the data is valid before using it
    if (gaze_point->validity == TOBII_VALIDITY_VALID) {

       /* printf("Gaze point: %f, %f\n",
            gaze_point->position_xy[0],
            gaze_point->position_xy[1]);*/
        x = gaze_point->position_xy[0];
        y = gaze_point->position_xy[1];
        if (x >= 0.4) command = "4";
        else command = "6";
        xy = to_string(x)+" "+to_string(y);
    }

}

void url_receiver(char const* url, void* user_data)
{
    char* buffer = (char*)user_data;
    if (*buffer != '\0') return; // only keep first value

    if (strlen(url) < 256)
        strcpy(buffer, url);
}

int main()
{
    //start
    SerialPort arduino(port);
    if (arduino.isConnected()) {
        cout << "Connection made" << endl << endl;
    }
    else {
        cout << "Error in port name" << endl << endl;
    }
    while (!arduino.isConnected()) {
        cout << "connection error" << endl;
    }

    //end
    // Create API
    tobii_api_t* api = NULL;
    tobii_error_t result = tobii_api_create(&api, NULL, NULL);
    assert(result == TOBII_ERROR_NO_ERROR);

    // Enumerate devices to find connected eye trackers, keep the first
    char url[256] = { 0 };
    result = tobii_enumerate_local_device_urls(api, url_receiver, url);
    assert(result == TOBII_ERROR_NO_ERROR);
    if (*url == '\0')
    {
        printf("Error: No device found\n");
        return 1;
    }

    // Connect to the first tracker found
    tobii_device_t* device = NULL;
    result = tobii_device_create(api, url, TOBII_FIELD_OF_USE_INTERACTIVE, &device);
    assert(result == TOBII_ERROR_NO_ERROR);

    // Subscribe to gaze data
    result = tobii_gaze_point_subscribe(device, gaze_point_callback, 0);
    assert(result == TOBII_ERROR_NO_ERROR);

    // This sample will collect 1000 gaze points
    for (int i = 0; i < 1000; i++)
    {
        // Optionally block this thread until data is available. Especially useful if running in a separate thread.
        tobii_wait_for_callbacks(1, &device);
        //assert(result == TOBII_ERROR_NO_ERROR || result == TOBII_ERROR_TIMED_OUT);

        // Process callbacks on this thread if data is available
        tobii_device_process_callbacks(device);

        string data;
        data=command;
        //cout << data << endl;

        char* charArray = new char[data.size() + 1];
        copy(data.begin(), data.end(), charArray);
        charArray[data.size()] = '\n';
        //cout << charArray[0] << endl;
        arduino.writeSerialPort(charArray, 20);
        //arduino.readSerialPort(output, MAX_DATA_LENGTH);

        //cout << ">> " << output << endl;

        delete[] charArray;


        std::cout <<xy<< std::endl;
        //assert(result == TOBII_ERROR_NO_ERROR);
    }

    // Cleanup
    tobii_gaze_point_unsubscribe(device);
    //assert(result == TOBII_ERROR_NO_ERROR);
    tobii_device_destroy(device);
    //assert(result == TOBII_ERROR_NO_ERROR);
    tobii_api_destroy(api);
    //assert(result == TOBII_ERROR_NO_ERROR);
    return 0;
}
