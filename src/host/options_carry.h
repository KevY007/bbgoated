#pragma once
// bbgoated: bbhost-options.toml (F10 settings, key bindings, the account) is shared
// with bbhost, whose save() rewrites the whole file from the keys it knows. Carried
// through a bbgoated save: every key or section the new file lacks - what a newer
// bbhost added - except the account when the new file has none (signed out stays
// signed out) and its name and token, which save() owns.
#include <string>

// The text bbgoated's save() wrote (new_text) plus what only the old file had.
std::string options_carry_unknown(const std::string& old_text, const std::string& new_text);

// options_carry_unknown on a file: path holds new_text now; old_text is what it held
// before this save. Rewrites it only when something was carried.
void options_carry_apply(const std::string& path, const std::string& old_text);
