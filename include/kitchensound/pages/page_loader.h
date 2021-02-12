#ifndef KITCHENSOUND_PAGE_LOADER_H
#define KITCHENSOUND_PAGE_LOADER_H

#include <memory>
#include <unordered_map>

#include "kitchensound/pages/pages.h"

class BasePage;
class Configuration;
class StateController;
class Volume;
class ResourceManager;
class GpioUtil;

std::unordered_map<PAGES, std::unique_ptr<BasePage>> load_pages(Configuration& conf,
                                                                StateController& ctrl,
                                                                ResourceManager& res,
                                                                Volume& vol);

#endif //KITCHENSOUND_PAGE_LOADER_H
