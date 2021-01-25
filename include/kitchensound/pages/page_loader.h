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

std::unordered_map<PAGES, std::unique_ptr<BasePage>> load_pages(std::unique_ptr<Configuration>& conf,
                                                                std::shared_ptr<StateController>& ctrl,
                                                                std::shared_ptr<ResourceManager>& res,
                                                                std::shared_ptr<Volume>& vol);

#endif //KITCHENSOUND_PAGE_LOADER_H
