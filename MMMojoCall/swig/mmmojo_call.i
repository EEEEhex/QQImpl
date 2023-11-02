%module mmmojo_call
%{
#include "mmmojo_call.h"
#include "mmmojo_ocr.h"
#include "mmmojo_utility.h"
%}

/* Parse the header file to generate wrappers */
%include <windows.i>

%include "../src/include/mmmojo_source/mmmojo.h"
%include "../src/include/mmmojo_source/mmmojo_environment_callbacks.h"
%include "../src/include/mojo_call_export.h"
%include "../src/include/mmmojo_call.h"
%include "../src/include/mmmojo_ocr.h"

%ignore qqimpl::mmmojocall::UtilityManager::DoQRScan;
%ignore qqimpl::mmmojocall::UtilityManager::DoDecodeImage;

%include "../src/include/mmmojo_utility.h"

