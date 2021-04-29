#pragma once

//#include<ATLComTime.h> 여기에 이것 들어가면 std::numeric_limits<int>::max() 에서 애러 난다.
#include <io.h>
#include <filesystem>
#include <map>
#include <omp.h>
#include <string>
#include <thread>
#include <time.h>
#include <vector>

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include "device_launch_parameters.h" // cuda에서 정의된 키워드 포함

#include "bitmap_image.hpp"
#include "cpuinfo.h"
#include "cpuinfodelegate.h"
#include "gpuinfo.h"
#include "gpuinfodelegate.h"

#include "systemcommand.h"

