// Copyright 2025 Xenon Emulator Project. All rights reserved.

#include "Core/XeMain.h"
#include "Base/Param.h"
#include "Base/Exit.h"
#include "Base/Thread.h"

volatile int hupflag = 0;
// Clean shutdown when we are sent by the OS to shutdown
s32 globalShutdownHandler() {
  // If we have been told we cannot safely terminate, just force exit without cleanup
  // The OS will need to handle it, but better than deadlocking the process
  if (XePaused) {
    const s32 exitCode = Base::exit(-1);
    return exitCode; // Avoid cleanup, as we cannot ensure it'll be done properly
  }
  // If we tried to exit gracefully the first time and failed,
  // use fexit to forcefully send a SIGTERM
  if (!hupflag) {
    printf("\nAttempting to clean shutdown...\n");
    hupflag = 1;
  } else {
    printf("\nUnable to clean shutdown!\n");
    printf("Press Crtl+C again to forcefully exit...\n");
    const s32 exitCode = Base::fexit(-1);
    return exitCode;
  }
  // Cleanly shutdown without the exit syscall
  XeRunning = false;
  // Give everything a while to shut down. If it still hasn't shutdown, then something hung
  std::this_thread::sleep_for(15s);
  if (XeShutdonSignaled) {
    printf("This was called because after 15s, and a shutdown call, it still hasn't shutdown.\n");
    printf("Something likely hung. If you have issues, please make a GitHub Issue report with this message in it\n");
    // We should force exit. We only should call shutdown once, and if we don't, then it hung
    return Base::exit(-1);
  } else {
    XeMain::Shutdown();
  }
  return 0;
}
#ifdef _WIN32
BOOL WINAPI consoleControlHandler(ul32 ctrlType) {
  switch (ctrlType) {
  case CTRL_C_EVENT:
  case CTRL_CLOSE_EVENT:
    return globalShutdownHandler() == 0 ? TRUE : FALSE; // Signal handled, prevent default behavior if told so
  }
  return FALSE; // Default handling
}
s32 installHangup() {
  return SetConsoleCtrlHandler(consoleControlHandler, TRUE) ? 0 : -1;
}
s32 removeHangup() {
  return SetConsoleCtrlHandler(consoleControlHandler, FALSE) ? 0 : -1;
}
#elif defined(__linux__)
#include <signal.h>
extern "C" void hangup(s32) {
  // Should this be handled here, if we deadlock our main thread?
  (void)globalShutdownHandler();
}
s32 installHangup() {
  struct sigaction act {};
  act.sa_handler = hangup;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (sigaction(SIGHUP, &act, nullptr) < 0) {
    return -1;
  }
  if (sigaction(SIGINT, &act, nullptr) < 0) {
    return -1;
  }
  if (sigaction(SIGTERM, &act, nullptr) < 0) {
    return -1;
  }
  if (sigaction(SIGHUP, &act, nullptr) < 0) {
    return -1;
  }
  return 0;
}
s32 removeHangup() {
  return 0;
}
#else
// I have zero clue if macOS handles it in the same way as Linux does
// Better to not handle it, until macOS is fully supported by Xenon
s32 installHangup() {
  return 0;
}
s32 removeHangup() {
  return 0;
}
#endif

PARAM(help, "Prints this message", false);

#define AUTO_FLIP 1
s32 main(s32 argc, char *argv[]) {
  MicroProfileOnThreadCreate("Main");
  // Init params
  Base::Param::Init(argc, argv);
  // Handle help param
  if (PARAM_help.Present()) {
    ::Base::Param::Help();
    return 0;
  }
  // Enable profiling
  MicroProfileSetEnableAllGroups(true);
  MicroProfileSetForceMetaCounters(true);
#if AUTO_FLIP
  MicroProfileStartAutoFlip(30);
#endif
  // Set thread name
  Base::SetCurrentThreadName("[Xe] Main");
  // Define profiler
  {
    // Create all handles
    XeMain::Create();
    // Setup hangup
    if (installHangup() != 0) {
      LOG_CRITICAL(System, "Failed to install signal handler. Clean shutdown is not possible through console");
    }
    // Start execution of the emulator
    XeMain::StartCPU();
  }
  // Inf wait until told otherwise
  while (XeRunning) {
#if MICROPROFILE_ENABLED == 1 && AUTO_FLIP == 0
    MicroProfileFlip(nullptr);
#endif
#ifndef NO_GFX
    if (XeMain::renderer.get())
      XeMain::renderer->HandleEvents();
#endif
    std::this_thread::sleep_for(100ms);
  }
  // Shutdown
  XeMain::Shutdown();
  return 0;
}
