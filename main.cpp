#include <iostream>
#include <Windows.h>
#include <vector>

uint32_t getVolumeClusterCount(const std::string& volumePath) {
  DWORD sectorsPerCluster;
  DWORD bytesPerSector;
  DWORD numbersOfFreeClusters;
  DWORD totalNumberOfClusters;

  GetDiskFreeSpaceA(
    volumePath.c_str(),
    &sectorsPerCluster,
    &bytesPerSector,
    &numbersOfFreeClusters,
    &totalNumberOfClusters
  );

  return totalNumberOfClusters;
}

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

VOLUME_BITMAP_BUFFER* getVolumeBitmap(const HANDLE volumeHandle, uint32_t& bitmapSize) {
  STARTING_LCN_INPUT_BUFFER sStartLcn{
    .StartingLcn = 0
  };

  DWORD dwBitmapSize = 0;
  DWORD dwAllocatedSize = 64 * 1024;
  VOLUME_BITMAP_BUFFER* pVolumeBitmap = NULL;
  while(true) {
    pVolumeBitmap = reinterpret_cast<VOLUME_BITMAP_BUFFER*>(new uint8_t[dwAllocatedSize]);

    BOOL result = DeviceIoControl(
      volumeHandle,
      FSCTL_GET_VOLUME_BITMAP,
      &sStartLcn,
      sizeof(sStartLcn),
      pVolumeBitmap,
      dwAllocatedSize,
      &dwBitmapSize,
      NULL
    );

    if(result) {
      bitmapSize = dwBitmapSize;
      return pVolumeBitmap;
    }

    if(GetLastError() != ERROR_MORE_DATA) {
      delete[] pVolumeBitmap;
      return nullptr;
    }

    delete[] pVolumeBitmap;

    dwAllocatedSize *= 2;
  }
}

bool getBitmapValue(PVOLUME_BITMAP_BUFFER bitmap, const uintptr_t index) {
  return (bitmap->Buffer[index / 8] >> index % 8) & 1;
}

int main(int argc, char** argv) {
  HANDLE volumeHandle = CreateFileA(
    "\\\\.\\G:",
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
  );

  if(volumeHandle == INVALID_HANDLE_VALUE) {
    std::cout << "Failed to open volume.\nError code: " << GetLastError() << std::endl;
    DebugBreak();
    return EXIT_FAILURE;
  }

  uint32_t bitmapSize = 0;
  VOLUME_BITMAP_BUFFER* volumeBitmap = getVolumeBitmap(volumeHandle, bitmapSize);

  for(uintptr_t i = 0; i < 500; ++i) {
    for(uintptr_t j = 0; j < 100; ++j) {
      float index = bitmapSize / (500.F * 100.F) * (j + i * 100.F);

      std::cout.put(getBitmapValue(volumeBitmap, static_cast<uintptr_t>(index)) ? '#' : '-');
    }
    std::cout.put('\n');
  }

  std::cout.flush();

  delete[] volumeBitmap;

  return EXIT_SUCCESS;
}
