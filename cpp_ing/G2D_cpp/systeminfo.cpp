// This source code was referenced from https://github.com/tlewiscpp/SystemInfo
#include "cpuinfo.h"
#include "cpuinfodelegate.h" 
#include "gpuinfo.h"
#include "gpuinfodelegate.h"

const std::string CPUInfo::TEMPERATURE_QUERY_STRING = "wmic /namespace:\\\\root\\wmi PATH MSAcpi_ThermalZoneTemperature get CurrentTemperature";
const std::string CPUInfo::TEMPERATURE_ERROR_IDENTIFIER_STRING = "ERROR:";
const std::string CPUInfo::CLOCK_SPEED_QUERY_STRING = "wmic cpu get /format:list | findstr /R /C:CurrentClockSpeed=";
const std::string CPUInfo::CURRENT_CLOCK_SPEED_IDENTIFIER_STRING = "CurrentClockSpeed=";
const std::string CPUInfo::NAME_IDENTIFIER_STRING = "Name=";
const std::string CPUInfo::NUMBER_OF_CORES_IDENTIFIER_STRING = "NumberOfCores=";
const std::string CPUInfo::MANUFACTURER_IDENTIFIER_STRING = "Manufacturer=";
const std::string CPUInfo::ARCHITECTURE_IDENTIFIER_STRING = "DataWidth=";
const std::string CPUInfo::L2_CACHE_SIZE_IDENTIFIER_STRING = "L2CacheSize=";
const std::string CPUInfo::L3_CACHE_SIZE_IDENTIFIER_STRING = "L3CacheSize=";

const std::string CPUInfoDelegate::CPU_INFO_QUERY_STRING = "wmic cpu get /format: list";
const std::string CPUInfoDelegate::CPU_INSTANCE_QUERY_STRING = "AssetTag=";
const std::string CPUInfoDelegate::CPU_INFO_END_IDENTIFIER_STRING = "VoltageCaps=";

const std::string GPUInfo::NVIDIA_IDENTIFIER_STRING = "NVIDIA";
const std::string GPUInfo::INTEL_IDENTIFIER_STRING = "INTEL";
const std::string GPUInfo::AMD_IDENTIFIER_STRING = "AMD";
const std::string GPUInfo::NAME_IDENTIFIER_STRING = "Name=";
const std::string GPUInfo::MANUFACTURER_IDENTIFIER_STRING = "AdapterCompatibility=";
const std::string GPUInfo::ADAPTER_RAM_IDENTIFIER_STRING = "AdapterRAM=";
const std::string GPUInfo::REFRESH_RATE_IDENTIFIER_STRING = "CurrentRefreshRate=";
const std::string GPUInfo::DRIVER_VERSION_IDENTIFIER_STRING = "DriverVersion=";
const std::string GPUInfo::VIDEO_ARCHITECTURE_IDENTIFIER_STRING = "VideoArchitecture=";
const std::string GPUInfo::VIDEO_MEMORY_TYPE_IDENTIFIER_STRING = "VideoMemoryType";
const std::string GPUInfo::VIDEO_MODE_DESCRIPTION_IDENTIFIER_STRING = "VideoModeDescription=";
const std::string GPUInfo::VIDEO_PROCESSOR_IDENTIFIER_STRING = "VideoProcessor=";
const std::string GPUInfo::CAPTION_IDENTIFIER_STRING = "Caption=";

const std::string GPUInfoDelegate::GPU_INFO_QUERY_STRING = "wmic path Win32_VideoController get /format: list";
const std::string GPUInfoDelegate::GPU_INSTANCE_QUERY_STRING = "CurrentBitsPerPixel=";
const std::string GPUInfoDelegate::GPU_INFO_END_IDENTIFIER_STRING = "VideoProcessor=";


CPUInfo::CPUInfo(const std::vector<std::string> &rawData, int cpuNumber) :
	_name{ "" },
	_manufacturer{ "" },
	_numberOfCores{ "" },
	_architecture{ "" },
	_L2CacheSize{ "" },
	_L3CacheSize{ "" },
	_cpuNumber{ cpuNumber }
{
	for (auto iter = rawData.begin(); iter != rawData.end(); iter++) {

		//Name 
		if ((iter->find(NAME_IDENTIFIER_STRING) != std::string::npos) && (iter->find(NAME_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(NAME_IDENTIFIER_STRING);
			this->_name = iter->substr(foundPosition + NAME_IDENTIFIER_STRING.length());
		}

		//Manufacturer 
		if ((iter->find(MANUFACTURER_IDENTIFIER_STRING) != std::string::npos) && (iter->find(MANUFACTURER_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(MANUFACTURER_IDENTIFIER_STRING);
			this->_manufacturer = iter->substr(foundPosition + MANUFACTURER_IDENTIFIER_STRING.length());
		}


		//Number Of Cores 
		if ((iter->find(NUMBER_OF_CORES_IDENTIFIER_STRING) != std::string::npos) && (iter->find(NUMBER_OF_CORES_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(NUMBER_OF_CORES_IDENTIFIER_STRING);
			this->_numberOfCores = iter->substr(foundPosition + NUMBER_OF_CORES_IDENTIFIER_STRING.length());
		}


		//Architecture 
		if ((iter->find(ARCHITECTURE_IDENTIFIER_STRING) != std::string::npos) && (iter->find(ARCHITECTURE_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(ARCHITECTURE_IDENTIFIER_STRING);
			std::string dataWidth = iter->substr(foundPosition + ARCHITECTURE_IDENTIFIER_STRING.length());
			this->_architecture = getArchitecture(dataWidth);
		}


		//L2 Cache Size 
		if ((iter->find(L2_CACHE_SIZE_IDENTIFIER_STRING) != std::string::npos) && (iter->find(L2_CACHE_SIZE_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(L2_CACHE_SIZE_IDENTIFIER_STRING);
			this->_L2CacheSize = iter->substr(foundPosition + L2_CACHE_SIZE_IDENTIFIER_STRING.length()) + "KB";
			if (this->_L2CacheSize == "KB") {
				this->_L2CacheSize = "";
			}
		}


		//L3 Cache Size 
		if ((iter->find(L3_CACHE_SIZE_IDENTIFIER_STRING) != std::string::npos) && (iter->find(L3_CACHE_SIZE_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(L3_CACHE_SIZE_IDENTIFIER_STRING);
			this->_L3CacheSize = iter->substr(foundPosition + L3_CACHE_SIZE_IDENTIFIER_STRING.length()) + "KB";
			if (this->_L3CacheSize == "KB") {
				this->_L3CacheSize = "";
			}
		}
	}
	//In case any of these values are missing or don't get assigned 
	if (this->_name == "") {
		this->_name = "Unknown";
	}
	if (this->_manufacturer == "") {
		this->_manufacturer = "Unknown";
	}
	if (this->_numberOfCores == "") {
		this->_numberOfCores = "Unknown";
	}
	if (this->_architecture == "") {
		this->_architecture = "Unknown";
	}
	if (this->_L2CacheSize == "") {
		this->_L2CacheSize = "Unknown";
	}
	if (this->_L3CacheSize == "") {
		this->_L3CacheSize = "Unknown";
	}
}


std::string CPUInfo::name() const
{
	return this->_name;
}


std::string CPUInfo::manufacturer() const
{
	return this->_manufacturer;
}


std::string CPUInfo::numberOfCores() const
{
	return this->_numberOfCores;
}


std::string CPUInfo::architecture() const
{
	return this->_architecture;
}


std::string CPUInfo::L2CacheSize() const
{
	return this->_L2CacheSize;
}


std::string CPUInfo::L3CacheSize() const
{
	return this->_L3CacheSize;
}


int CPUInfo::cpuNumber() const
{
	return this->_cpuNumber;
}


std::string CPUInfo::currentClockSpeed() const
{
	std::string clockSpeed{ "" };
	SystemCommand systemCommand{ CLOCK_SPEED_QUERY_STRING };
	systemCommand.execute();
	if (!systemCommand.hasError()) {
		std::vector<std::string> raw{ systemCommand.outputAsVector() };
		if (raw.empty()) {
			clockSpeed = "Unknown";
		}
		int cpuInfoNumber = 0;
		for (std::vector<std::string>::const_iterator iter = raw.begin(); iter != raw.end(); iter++) {
			if (cpuInfoNumber == this->_cpuNumber) {
				if ((iter->find(CURRENT_CLOCK_SPEED_IDENTIFIER_STRING) != std::string::npos) && (iter->find(CURRENT_CLOCK_SPEED_IDENTIFIER_STRING) == 0)) {
					size_t foundPosition = iter->find(CURRENT_CLOCK_SPEED_IDENTIFIER_STRING);
					clockSpeed = iter->substr(foundPosition + CURRENT_CLOCK_SPEED_IDENTIFIER_STRING.length()) + "MHz";
				}
			}
			cpuInfoNumber++;
		}
	}
	else {
		clockSpeed = "Unknown";
	}
	if ((clockSpeed == "MHz") || (clockSpeed == "")) {
		clockSpeed = "Unknown";
	}
	return clockSpeed;
}


std::string CPUInfo::currentTemperature() const
{
	//NOTE: THIS IS NOT SUPPORTED BY ALL COMPUTERS!!! 
	std::string temperature{ "" };
	SystemCommand systemCommand{ TEMPERATURE_QUERY_STRING };
	systemCommand.execute();
	if (!systemCommand.hasError()) {
		std::vector<std::string> raw{ systemCommand.outputAsVector() };
		for (auto iter = raw.begin(); iter != raw.end(); iter++) {
			if (iter->find(TEMPERATURE_ERROR_IDENTIFIER_STRING)) {
				temperature = "Unknown";
			}
			else {
				std::string rawTemp = *(raw.begin() + 1);
				try {
					int tempInKelvin = std::stoi(rawTemp);
					int tempInCelcius = kelvinToCelcius(tempInKelvin);
					temperature = toString(tempInCelcius) + "C";
				}
				catch (std::exception &e) {
					(void)e;
					temperature = "Unknown";
				}
			}
		}
	}
	else {
		temperature = "Unknown";
	}
	return temperature;
}




int CPUInfo::kelvinToCelcius(int tempInKelvin) const
{
	return tempInKelvin - 273;
}


std::string CPUInfo::getArchitecture(std::string &dataWidth) const
{
	try {
		int dataWidthInt = std::stoi(dataWidth);
		switch (dataWidthInt) {
		case 32: return "x86";
		case 64: return "x86_64";
		default: return "Unknown";
		}
	}
	catch (std::exception &e) {
		(void)e;
		return "Unknown";
	}
}

CPUInfoDelegate::CPUInfoDelegate() :
	_numberOfCPUInfoItems{ 0 }
{
	SystemCommand systemCommand{ CPU_INFO_QUERY_STRING };
	systemCommand.execute();
	std::vector<std::string> tempVector = systemCommand.outputAsVector();
	if (!systemCommand.hasError()) {
		std::vector<std::string> raw = { systemCommand.outputAsVector() };
		determineNumberOfCPUInfoItems(raw);
		std::vector<std::string> singleCPUInfoItem;
		std::vector<std::string>::const_iterator iter = raw.begin();
		int cpuNumber = 0;
		while (cpuNumber < this->_numberOfCPUInfoItems) {
			while (iter->find(CPU_INFO_END_IDENTIFIER_STRING) == std::string::npos) {
				if ((*iter != "") && (*iter != "\r")) {
					singleCPUInfoItem.push_back(*iter);
				}
				iter++;
			}
			singleCPUInfoItem.push_back(*iter);
			this->_cpuInfoVector.emplace_back(singleCPUInfoItem, cpuNumber);
			singleCPUInfoItem.clear();
			iter++;
			cpuNumber++;
		}
	}
}


void CPUInfoDelegate::determineNumberOfCPUInfoItems(const std::vector<std::string> &data)
{
	for (auto iter = data.begin(); iter != data.end(); iter++) {
		if (iter->find(CPU_INSTANCE_QUERY_STRING) != std::string::npos) {
			this->_numberOfCPUInfoItems++;
		}
	}
}


int CPUInfoDelegate::numberOfCPUInfoItems() const
{
	return this->_numberOfCPUInfoItems;
}


std::vector<CPUInfo> CPUInfoDelegate::cpuInfoVector() const
{
	return this->_cpuInfoVector;
}


GPUInfo::GPUInfo(const std::vector<std::string> &rawData, int gpuNumber) :
	_name{ "" },
	_manufacturer{ "" },
	_caption{ "" },
	_adapterRAM{ "" },
	_refreshRate{ "" },
	_driverVersion{ "" },
	_videoArchitecture{ "" },
	_videoMemoryType{ "" },
	_videoModeDescription{ "" },
	_videoProcessor{ "" },
	_gpuNumber{ gpuNumber }
{
	for (auto iter = rawData.begin(); iter != rawData.end(); iter++) {


		//Name 
		if ((iter->find(NAME_IDENTIFIER_STRING) != std::string::npos) && (iter->find(NAME_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(NAME_IDENTIFIER_STRING);
			this->_name = iter->substr(foundPosition + NAME_IDENTIFIER_STRING.length());
		}


		//Manufacturer 
		if ((iter->find(MANUFACTURER_IDENTIFIER_STRING) != std::string::npos) && (iter->find(MANUFACTURER_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(MANUFACTURER_IDENTIFIER_STRING);
			this->_manufacturer = iter->substr(foundPosition + MANUFACTURER_IDENTIFIER_STRING.length());
		}


		//Caption 
		if ((iter->find(CAPTION_IDENTIFIER_STRING) != std::string::npos) && (iter->find(CAPTION_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(CAPTION_IDENTIFIER_STRING);
			this->_caption = iter->substr(foundPosition + CAPTION_IDENTIFIER_STRING.length());
		}


		//Adapter RAM 
		if ((iter->find(ADAPTER_RAM_IDENTIFIER_STRING) != std::string::npos) && (iter->find(ADAPTER_RAM_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(ADAPTER_RAM_IDENTIFIER_STRING);
			std::string capacityString = iter->substr(foundPosition + ADAPTER_RAM_IDENTIFIER_STRING.length());
			long long int capacity{ 0 };
			try {
				capacity = std::stoll(capacityString);
				this->_adapterRAM = toString(capacity / 1000000) + "MB (" + toString(capacity) + " Bytes)";
			}
			catch (std::exception &e) {
				(void)e;
				this->_adapterRAM = capacityString + " Bytes";
			}
		}


		//Refresh Rate 
		if ((iter->find(REFRESH_RATE_IDENTIFIER_STRING) != std::string::npos) && (iter->find(REFRESH_RATE_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(REFRESH_RATE_IDENTIFIER_STRING);
			this->_refreshRate = iter->substr(foundPosition + REFRESH_RATE_IDENTIFIER_STRING.length()) + "MHz";
			if (this->_refreshRate == "MHz") {
				this->_refreshRate = "";
			}
		}


		//Driver Version 
		if ((iter->find(DRIVER_VERSION_IDENTIFIER_STRING) != std::string::npos) && (iter->find(DRIVER_VERSION_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(DRIVER_VERSION_IDENTIFIER_STRING);
			this->_driverVersion = iter->substr(foundPosition + DRIVER_VERSION_IDENTIFIER_STRING.length());
		}


		//Video Architecture 
		if ((iter->find(VIDEO_ARCHITECTURE_IDENTIFIER_STRING) != std::string::npos) && (iter->find(VIDEO_ARCHITECTURE_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(VIDEO_ARCHITECTURE_IDENTIFIER_STRING);
			std::string videoArchitectureString = iter->substr(foundPosition, VIDEO_ARCHITECTURE_IDENTIFIER_STRING.length());
			this->_videoArchitecture = getVideoArchitecture(videoArchitectureString);
		}


		//Video Memory Type 
		if ((iter->find(VIDEO_MEMORY_TYPE_IDENTIFIER_STRING) != std::string::npos) && (iter->find(VIDEO_MEMORY_TYPE_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(VIDEO_MEMORY_TYPE_IDENTIFIER_STRING);
			std::string videoMemoryTypeString = iter->substr(foundPosition, VIDEO_MEMORY_TYPE_IDENTIFIER_STRING.length());
			this->_videoMemoryType = getVideoMemoryType(videoMemoryTypeString);
		}


		//Video Mode Description 
		if ((iter->find(VIDEO_MODE_DESCRIPTION_IDENTIFIER_STRING) != std::string::npos) && (iter->find(VIDEO_MODE_DESCRIPTION_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(VIDEO_MODE_DESCRIPTION_IDENTIFIER_STRING);
			this->_videoModeDescription = iter->substr(foundPosition + VIDEO_MODE_DESCRIPTION_IDENTIFIER_STRING.length());
		}


		//Video Processor 
		if ((iter->find(VIDEO_PROCESSOR_IDENTIFIER_STRING) != std::string::npos) && (iter->find(VIDEO_PROCESSOR_IDENTIFIER_STRING) == 0)) {
			size_t foundPosition = iter->find(VIDEO_PROCESSOR_IDENTIFIER_STRING);
			this->_videoProcessor = iter->substr(foundPosition + VIDEO_PROCESSOR_IDENTIFIER_STRING.length());
		}
	}
	//In case any of these values are missing or don't get assigned 
	if (this->_name == "") {
		this->_name = "Unknown";
	}
	if (this->_manufacturer == "") {
		this->_manufacturer = "Unknown";
	}
	if (this->_caption == "") {
		this->_caption = "Unknown";
	}
	if (this->_adapterRAM == "") {
		this->_adapterRAM = "Unknown";
	}
	if (this->_refreshRate == "") {
		this->_refreshRate = "Unknown";
	}
	if (this->_driverVersion == "") {
		this->_driverVersion = "Unknown";
	}
	if (this->_videoArchitecture == "") {
		this->_videoArchitecture = "Unknown";
	}
	if (this->_videoMemoryType == "") {
		this->_videoMemoryType = "Unknown";
	}
	if (this->_videoModeDescription == "") {
		this->_videoModeDescription = "Unknown";
	}
	if (this->_videoProcessor == "") {
		this->_videoProcessor = "Unknown";
	}
}


std::string GPUInfo::getVideoArchitecture(const std::string &videoArchitectureString) const
{
	int videoArch{ 2 };
	try {
		videoArch = std::stoi(videoArchitectureString);
	}
	catch (std::exception &e) {
		(void)e;
		videoArch = 2;
	}
	//As per https://msdn.microsoft.com/en-us/library/aa394512(v=vs.85).aspx 
	switch (videoArch) {
	case 1: return "Other";
	case 2: return "Unknown";
	case 3: return "CGA";
	case 4: return "EGA";
	case 5: return "VGA";
	case 6: return "SVGA";
	case 7: return "MDA";
	case 8: return "HGC";
	case 9: return "MCGA";
	case 10: return "8514A";
	case 11: return "XGA";
	case 12: return "Linear Frame Buffer";
	case 160: return "PC - 98";
	default: return "Unknown";
	}
}


std::string GPUInfo::getVideoMemoryType(const std::string &videoMemoryTypeString) const
{
	int videoMemoryType{ 2 };
	try {
		videoMemoryType = std::stoi(videoMemoryTypeString);
	}
	catch (std::exception &e) {
		(void)e;
		videoMemoryType = 2;
	}
	switch (videoMemoryType) {
	case 1: return "Other";
	case 2: return "Unknown";
	case 3: return "VRAM";
	case 4: return "DRAM";
	case 5: return "SRAM";
	case 6: return "WRAM";
	case 7: return "EDO_RAM";
	case 8: return "Burst Synchronous DRAM";
	case 9: return "Pipelined Burst SRAM";
	case 10: return "CDRAM";
	case 11: return "3DRAM";
	case 12: return "SDRAM";
	case 13: return "SGRAM";
	default: return "Unknown";
	}
}


std::string GPUInfo::name() const
{
	return this->_name;
}


std::string GPUInfo::manufacturer() const
{
	return this->_manufacturer;
}


std::string GPUInfo::caption() const
{
	return this->_caption;
}


std::string GPUInfo::adapterRAM() const
{
	return this->_adapterRAM;
}


std::string GPUInfo::refreshRate() const
{
	return this->_refreshRate;
}


std::string GPUInfo::driverVersion() const
{
	return this->_driverVersion;
}


std::string GPUInfo::videoArchitecture() const
{
	return this->_videoArchitecture;
}


std::string GPUInfo::videoProcessor() const
{
	return this->_videoProcessor;
}


std::string GPUInfo::videoMemoryType() const
{
	return this->_videoMemoryType;
}


std::string GPUInfo::videoModeDescription() const
{
	return this->_videoModeDescription;
}


int GPUInfo::gpuNumber() const
{
	return this->_gpuNumber;
}

GPUInfoDelegate::GPUInfoDelegate() :
	_numberOfGPUInfoItems{ 0 }
{
	SystemCommand systemCommand{ GPU_INFO_QUERY_STRING };
	systemCommand.execute();
	std::vector<std::string> tempVector = systemCommand.outputAsVector();
	if (!systemCommand.hasError()) {
		std::vector<std::string> raw = { systemCommand.outputAsVector() };
		determineNumberOfGPUInfoItems(raw);
		std::vector<std::string> singleGPUInfoItem;
		std::vector<std::string>::const_iterator iter = raw.begin();
		int gpuNumber = 0;
		while (gpuNumber < this->_numberOfGPUInfoItems) {
			while (iter->find(GPU_INFO_END_IDENTIFIER_STRING) == std::string::npos) {
				if ((*iter != "") && (*iter != "\r")) {
					singleGPUInfoItem.push_back(*iter);
				}
				iter++;
			}
			singleGPUInfoItem.push_back(*iter);
			this->_gpuInfoVector.emplace_back(singleGPUInfoItem, gpuNumber);
			singleGPUInfoItem.clear();
			iter++;
			gpuNumber++;
		}
	}
}


void GPUInfoDelegate::determineNumberOfGPUInfoItems(const std::vector<std::string> &data)
{
	for (auto iter = data.begin(); iter != data.end(); iter++) {
		if (iter->find(GPU_INSTANCE_QUERY_STRING) != std::string::npos) {
			this->_numberOfGPUInfoItems++;
		}
	}
}

int GPUInfoDelegate::numberOfGPUInfoItems() const
{
	return this->_numberOfGPUInfoItems;
}


std::vector<GPUInfo> GPUInfoDelegate::gpuInfoVector() const
{
	return this->_gpuInfoVector;
}
