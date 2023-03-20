#include <iostream>
#include <Windows.h>
#include <vector>

uint64_t getVolumeSize(const HANDLE volumeHandle) {
  DISK_GEOMETRY_EX geometry;
  DWORD bytesReturned;

  BOOL result = DeviceIoControl(
    volumeHandle,
    IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
    NULL,
    0,
    &geometry,
    sizeof(geometry),
    &bytesReturned,
    NULL
  );

  if(!result) {
    DebugBreak();
  }

  return geometry.DiskSize.QuadPart;
}

void getVolumeBitmap(const HANDLE volumeHandle, std::vector<uint8_t>& bitmap) {
  uint64_t bytesWanted = getVolumeSize(volumeHandle);
  DWORD bytesReturned;

  bitmap.resize(bytesWanted);

  BOOL result = DeviceIoControl(
    volumeHandle,
    FSCTL_GET_VOLUME_BITMAP,
    NULL,
    0,
    bitmap.data(),
    bitmap.size(),
    &bytesReturned,
    NULL
  );

  if(!result) {
    DebugBreak();
  }
}

int main(int argc, char** argv) {
  char volumePath[128];
  GetVolumeNameForVolumeMountPointA(
    "G:\\",
    volumePath,
    sizeof(volumePath)
  );

  HANDLE volumeHandle = CreateFileA(
    volumePath,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
  );

  if(volumeHandle == INVALID_HANDLE_VALUE) {
    std::cout << "Error: " << GetLastError() << std::endl;
    DebugBreak();
    return EXIT_FAILURE;
  }

  std::vector<uint8_t> volumeBitmap;

  getVolumeBitmap(volumeHandle, volumeBitmap);

  return EXIT_SUCCESS;
}
