#pragma once

#include "types.h"

constexpr int64 OK = 0;
constexpr int64 ErrorFileNotFound = -4;
constexpr int64 ErrorPermissionDenied = -5;
constexpr int64 ErrorInvalidFD = -6;
constexpr int64 ErrorInvalidBuffer = -7;
constexpr int64 ErrorInvalidPath = -8;
constexpr int64 ErrorInvalidFileSystem = -9;
constexpr int64 ErrorInvalidDevice = -10;
constexpr int64 ErrorFileExists = -11;
constexpr int64 ErrorSeekOffsetOOB = -12;
constexpr int64 ErrorEncounteredSymlink = -13;
constexpr int64 ErrorAddressNotPageAligned = -14;
constexpr int64 ErrorPathTooLong = -15;
constexpr int64 ErrorHardlinkToFolder = -16;
constexpr int64 ErrorHardlinkToDifferentFS = -17;
constexpr int64 ErrorDeleteMountPoint = -18;
constexpr int64 ErrorFolderNotEmpty = -19;
constexpr int64 ErrorNotAFolder = -20;
constexpr int64 ErrorFolderAlreadyMounted = -21;
constexpr int64 ErrorNotABlockDevice = -22;
constexpr int64 ErrorNotAMountPoint = -23;
constexpr int64 ErrorUnmountRoot = -24;
constexpr int64 ErrorMountPointBusy = -25;
constexpr int64 ErrorOpenFolder = -26;
constexpr int64 ErrorNotADevice = -27;

constexpr int64 ErrorThreadNotFound = -100;
constexpr int64 ErrorDetachSubThread = -101;
constexpr int64 ErrorThreadNotExited = -102;

constexpr int64 ErrorInterrupted = -200;

constexpr int64 ErrorNotSupportOperator = -201;

const char* ErrorToString(int64 error);