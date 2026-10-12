#pragma once
// bbgoated (cmake/bbgoated.cmake, docs/bbgoated/UPSTREAM.md): what bbgoated
// adds outside the renderer. Built only with -DBBGOATED=ON.

// The window's title: a bbgoated window is told apart from bbhost's.
#if defined(BBHOST_BBGOATED)
#define BBHOST_WINDOW_TITLE "Bloodborne (bbgoated)"
#define BBHOST_PRODUCT "bbgoated"
#else
#define BBHOST_WINDOW_TITLE "Bloodborne (bbhost)"
#define BBHOST_PRODUCT "bbhost"
#endif

// Logs each running bbhost.exe: bbgoated signs in with the account it copied
// from bbhost, and two programs online with one account at once confuse the
// server's sessions.
void bbgoated_warn_other_instances();
