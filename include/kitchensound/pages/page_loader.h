#ifndef KITCHENSOUND_PAGE_LOADER_H
#define KITCHENSOUND_PAGE_LOADER_H

#include <memory>
#include <unordered_map>

#include "kitchensound/pages/pages.h"

struct ApplicationBackbone;
class BasePage;

std::unordered_map<PAGES, std::unique_ptr<BasePage>> load_pages(ApplicationBackbone&);

#endif //KITCHENSOUND_PAGE_LOADER_H
