#ifndef XENIA_CONFIG_H_
#define XENIA_CONFIG_H_
#include <string>

namespace config {
void SetupConfig(const std::u16string& config_folder);
void LoadGameConfig(const std::u16string& title_id);
}  // namespace config

#endif  // XENIA_CONFIG_H_
