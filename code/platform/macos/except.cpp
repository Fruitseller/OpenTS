/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "win.h"
#include "dbgprint.h"
#include "except.h"

#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <libproc.h>
#include <signal.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <thread>

namespace {

char CrashReportPath[MAX_PATH] = {};
char RegisteredLogFile[MAX_PATH] = {};
char TestMode[32] = {};

void Write_All(int file, char const * text, size_t length)
{
	while (length != 0) {
		ssize_t const written = write(file, text, length);
		if (written <= 0) return;
		text += written;
		length -= static_cast<size_t>(written);
	}
}

void Crash_Handler(int signal_number, siginfo_t * information, void *)
{
	int const file = open(CrashReportPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (file >= 0) {
		char header[512];
		int const length = snprintf(header, sizeof(header),
			"OpenTS macOS crash report\nSignal: %d\nAddress: %p\nDebug log: %s\n\nBacktrace:\n",
			signal_number, information ? information->si_addr : nullptr,
			RegisteredLogFile[0] ? RegisteredLogFile : "unavailable");
		if (length > 0) Write_All(file, header, static_cast<size_t>(length));
		void * frames[128];
		int const count = backtrace(frames, static_cast<int>(std::size(frames)));
		backtrace_symbols_fd(frames, count, file);
		close(file);
	}

	signal(signal_number, SIG_DFL);
	raise(signal_number);
}

[[noreturn]] void Terminate_Handler()
{
	raise(SIGABRT);
	_exit(EXIT_FAILURE);
}

__attribute__((noinline)) unsigned Test_Recurse(unsigned depth)
{
	volatile char block[4096];
	block[0] = static_cast<char>(depth);
	return Test_Recurse(depth + 1) + block[0];
}

void Fault()
{
	*(volatile int *)16 = 1;
}

} // namespace

void Install_Exception_Handler()
{
	char executable[PROC_PIDPATHINFO_MAXSIZE] = {};
	std::filesystem::path directory = ".";
	if (proc_pidpath(getpid(), executable, sizeof(executable)) > 0) {
		directory = std::filesystem::path(executable).parent_path();
	}
	std::error_code error;
	directory /= "Crash";
	std::filesystem::create_directories(directory, error);
	std::filesystem::path const report = directory / "crash-report.txt";
	strncpy(CrashReportPath, report.c_str(), sizeof(CrashReportPath) - 1);

	struct sigaction action = {};
	action.sa_sigaction = Crash_Handler;
	action.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sigemptyset(&action.sa_mask);
	for (int const crash_signal : {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV}) {
		sigaction(crash_signal, &action, nullptr);
	}
	std::set_terminate(Terminate_Handler);

	if (char const * requested = getenv("OPENTS_EXCEPTION_TEST")) {
		Exception_Set_Test_Mode(requested);
	}
}

void Exception_Register_Log_File(char const * path)
{
	RegisteredLogFile[0] = '\0';
	if (path) strncpy(RegisteredLogFile, path, sizeof(RegisteredLogFile) - 1);
}

void Exception_Set_Test_Mode(char const * mode)
{
	TestMode[0] = '\0';
	if (mode) strncpy(TestMode, mode, sizeof(TestMode) - 1);
}

void Exception_Run_Immediate_Test()
{
	if (TestMode[0] == '\0') return;
	if (stricmp(TestMode, "av-read") == 0 || stricmp(TestMode, "av-write") == 0
		|| stricmp(TestMode, "sectionfault") == 0) {
		Fault();
	} else if (stricmp(TestMode, "stack") == 0) {
		(void)Test_Recurse(0);
	} else if (stricmp(TestMode, "terminate") == 0 || stricmp(TestMode, "purecall") == 0
		|| stricmp(TestMode, "invalidparam") == 0) {
		std::terminate();
	} else if (stricmp(TestMode, "fatal") == 0) {
		Fatal("Requested test failure %d.", 1);
	} else if (stricmp(TestMode, "worker") == 0) {
		std::thread worker(Fault);
		worker.join();
	}
}

void Exception_Run_Post_Window_Test()
{
	if (stricmp(TestMode, "wndproc") == 0) {
		PostMessage(MainWindow, WM_EXCEPTION_TEST, 0, 0);
	} else if (stricmp(TestMode, "timer") == 0) {
		std::thread([]() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); Fault(); }).detach();
	}
}

void Exception_Wndproc_Test_Fault()
{
	Fault();
}

bool Describe_Code_Address(void const * address, char * buffer, unsigned size)
{
	if (!buffer || size == 0) return false;
	buffer[0] = '\0';
	Dl_info info;
	if (dladdr(address, &info) && info.dli_sname) {
		uintptr_t const displacement = (uintptr_t)address - (uintptr_t)info.dli_saddr;
		snprintf(buffer, size, "%s()+0x%lx", info.dli_sname, displacement);
		buffer[size - 1] = '\0';
		return true;
	}
	return false;
}
