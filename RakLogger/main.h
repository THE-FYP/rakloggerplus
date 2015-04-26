#include <windows.h>
#include <time.h>
#include <io.h>
#include <string>
#include <fstream>
#include <functional>
#include <regex>
#include <map>
#include <list>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
// #include <boost/filesystem.hpp>

#include "CSAMPFUNCS.h"
#include "types.h"
#include "config.h"
#include "functional.h"
#include "render.h"
#include "callbacks.h"

extern CSAMPFUNCS *SF;
extern CSAMP *SAMP;
extern CGame *Game;