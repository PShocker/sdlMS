#include "CrawlLine.h"

CrawlLine::CrawlLine(SDL_FPoint m, SDL_FPoint n)
{
    line = new Line(m, n);
}