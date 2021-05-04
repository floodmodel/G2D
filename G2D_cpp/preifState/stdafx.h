#pragma once

//#include<ATLComTime.h> ���⿡ �̰� ���� std::numeric_limits<int>::max() ���� �ַ� ����.
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
#include "device_launch_parameters.h" // cuda���� ���ǵ� Ű���� ����

#include "bitmap_image.hpp"
#include "cpuinfo.h"
#include "cpuinfodelegate.h"
#include "gpuinfo.h"
#include "gpuinfodelegate.h"

#include "systemcommand.h"

