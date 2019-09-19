#include <stdio.h>

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>  
#include <algorithm>

#include <Windows.h>
#include <TlHelp32.h>

#include "utils.h"

using namespace std;

static void sys_sleep(uint32_t nMilliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(nMilliseconds));
}

static std::wstring LowerCase(const std::wstring& text)
{
	std::wstring ret = text;
	std::transform(ret.begin(), ret.end(), ret.begin(), tolower);
	return ret;
}

static uint64_t GetDLLBaseAddress(std::wstring needDllName, int32_t pid)
{
	if (pid == 0) return 0;
	MODULEENTRY32 module;
	module.dwSize = sizeof(module);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		uint32_t error_code = GetLastError();
		return 0;
	}

	uint64_t ret = 0;

	BOOL bMore = Module32First(hProcessSnap, &module);
	int loaded_count = 0;
	while (bMore) {
		std::wstring dll_name = module.szModule;
		dll_name = LowerCase(dll_name);
		needDllName = LowerCase(needDllName);
		if (dll_name == needDllName) {
			ret = (uint64_t)module.modBaseAddr;
			break;
		}
		bMore = Module32Next(hProcessSnap, &module);
	}
	CloseHandle(hProcessSnap);

	return ret;
}

static std::vector<DWORD> GetPidByProcessName(wstring process_name) 
{ 

	std::vector<DWORD> ret;

	HANDLE hSnapshot; 
	PROCESSENTRY32W lppe; 
	hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapshot == NULL) 
		return ret; 

	lppe.dwSize = sizeof(lppe); 

	if (!::Process32FirstW(hSnapshot, &lppe))
		return ret;

	process_name = LowerCase(process_name);

	do { 
		wstring exeName = lppe.szExeFile;
		exeName = LowerCase(exeName);
		if(exeName == process_name) { 
			ret.push_back(lppe.th32ProcessID);
		}   
	} while (::Process32NextW(hSnapshot, &lppe));
	CloseHandle(hSnapshot);
	return ret;
}

static bool my_WriteProcessMemory(HANDLE h, uint64_t address, const uint8_t* byte, int byte_size)
{
	return WriteProcessMemory(h, (LPVOID)address, byte, byte_size, NULL);
}

#define  patch(i, b, b_size)  my_WriteProcessMemory(hopen, target_addr+i, b, b_size)

static std::wstring s_target_process_name = L"ChsIME.exe";

class win10_wubi_patch
{
public:
	win10_wubi_patch() 
	{
		_exit = false;
	}
	~win10_wubi_patch()
	{
		_exit = true;
		if (_thread.joinable()) {
			_thread.join();
		}
	}
	void RunThread()
	{
		_thread = std::thread(std::bind(&win10_wubi_patch::Execute, this));
	}
protected:
	virtual int32_t Execute(void)
	{
		uint32_t count = 0;
		while (!_exit) {

			sys_sleep(5000);

			std::vector<DWORD> pids = GetPidByProcessName(s_target_process_name);
			for (auto pid : pids) {
				if (_patched_pids.find(pid) == _patched_pids.end()) {
					count++;
					bool bOK = do_patch(pid);
					std::string s_bok = "success";
					if (bOK) {
						_patched_pids.insert(pid);
					} else {
						s_bok = "failed";
					}
					printf("---- do patch, pid = %d, times = %d, result = %s\n", pid, count, s_bok.c_str());
				}
			}
		}
		return 0;
	}
private:
	bool do_patch(int32_t pid)
	{

		bool bOK = false;
		HANDLE hopen = NULL;
		do 
		{
			hopen = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_TERMINATE | PROCESS_VM_OPERATION | PROCESS_VM_READ |
				PROCESS_VM_WRITE, FALSE, pid);
			if (hopen == NULL) {
				break;
			}

			uint64_t target_addr = GetDLLBaseAddress(s_target_process_name, pid);
			if (target_addr <= 0) {
				break;
			}

			///< patch here!
			uint8_t patch_bytes[] = {
				0x31, 0xC0
			};
			bOK = patch(0x14DE1, patch_bytes, sizeof(patch_bytes)); 
		} while (0);

		if (hopen) {
			CloseHandle(hopen);
			hopen = NULL;
		}

		return bOK;
	}
private:
	volatile bool _exit;
	std::set<DWORD> _patched_pids;
	std::thread _thread;
};

int main()
{

	if (!set_debug_privilege()) {
		printf("admin privileges are required.\n");
		return -1;
	}

	term_init();

	win10_wubi_patch wubi_patch;
	wubi_patch.RunThread();
	printf("---Widows 10 Disable English Switch Key(Shift) For Wubi InputMethod---  =key press [Q] to exit=\n");
	do {
		if (read_key() == 'q') {
			printf("exit\n");
			sys_sleep(500);
			break;
		}
		sys_sleep(10);
	} while (1);

	term_exit();
	return 0;
}