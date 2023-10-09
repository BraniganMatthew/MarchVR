#include <DeviceProvider.h>

EVRInitError DeviceProvider::Init(IVRDriverContext* pDriverContext)
{
    EVRInitError initError = InitServerDriverContext(pDriverContext);
    if (initError != EVRInitError::VRInitError_None)
    {
        return initError;
    }
    
    VRDriverLog()->Log("Initializing marchvr controller"); //this is how you log out Steam's log file.

    controllerDriver = new ControllerDriver();
    VRServerDriverHost()->TrackedDeviceAdded("marchvr_controller", TrackedDeviceClass_Controller, controllerDriver); //add all your devices like this.

    return vr::VRInitError_None;
}

void DeviceProvider::Cleanup()
{
    delete controllerDriver;
    controllerDriver = NULL;
}
const char* const* DeviceProvider::GetInterfaceVersions()
{
    return k_InterfaceVersions;
}

void DeviceProvider::RunFrame()
{
    controllerDriver->RunFrame();
}

bool DeviceProvider::ShouldBlockStandbyMode()
{
    return false;
}

void DeviceProvider::EnterStandby() {
    VRDriverLog()->Log("Entering Standby");
}

void DeviceProvider::LeaveStandby() {
    VRDriverLog()->Log("Leaving Standby");
}