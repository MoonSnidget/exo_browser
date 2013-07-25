// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#include "content/shell/shell_browser_main_parts.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "cc/base/switches.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"

#include "breach/browser/breach_browser_context.h"
#include "breach/common/breach_switches.h"
#include "breach/browser/browser.h"
/* TODO(spolu): renaming post file creation */
#include "content/shell/shell_devtools_delegate.h"
#include "content/shell/shell_net_log.h"

#include "grit/net_resources.h"
#include "net/base/net_module.h"
#include "net/base/net_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "url/gurl.h"

using namespace content;

namespace breach {

namespace {

static GURL 
GetStartupURL() 
{
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  const CommandLine::StringVector& args = command_line->GetArgs();

  if (args.empty())
    return GURL("http://www.google.com/");

  GURL url(args[0]);
  if (url.is_valid() && url.has_scheme())
    return url;

  return net::FilePathToFileURL(base::FilePath(args[0]));
}

base::StringPiece 
PlatformResourceProvider(
    int key) 
{
  if (key == IDR_DIR_HEADER_HTML) {
    base::StringPiece html_data =
        ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
            IDR_DIR_HEADER_HTML);
    return html_data;
  }
  return base::StringPiece();
}

}  // namespace

BreachBrowserMainParts::BreachBrowserMainParts(
    const MainFunctionParams& parameters)
    : BrowserMainParts(), 
      parameters_(parameters), 
      run_message_loop_(true) 
{
}

BreachBrowserMainParts::~BreachBrowserMainParts() {
}

#if !defined(OS_MACOSX)
void 
BreachBrowserMainParts::PreMainMessageLoopStart()
{
}
#endif

void 
BreachBrowserMainParts::PostMainMessageLoopStart() 
{
}

void 
BreachBrowserMainParts::PreEarlyInitialization() 
{
}

void 
BreachBrowserMainParts::PreMainMessageLoopRun() 
{
  /* TODO(spolu): renaming post file creation */
  net_log_.reset(new ShellNetLog());
  browser_context_.reset(new BreachBrowserContext(false, net_log_.get()));
  off_the_record_browser_context_.reset(
      new BreachBrowserContext(true, net_log_.get()));

  Browser::Initialize();
  net::NetModule::SetResourceProvider(PlatformResourceProvider);

  /* TODO(spolu): renaming post file creation */
  devtools_delegate_.reset(new ShellDevToolsDelegate(browser_context_.get()));

  if (parameters_.ui_task) {
    parameters_.ui_task->Run();
    delete parameters_.ui_task;
    run_message_loop_ = false;
  }
}

bool BreachBrowserMainParts::MainMessageLoopRun(int* result_code)  
{
  return !run_message_loop_;
}

void BreachBrowserMainParts::PostMainMessageLoopRun() 
{
  if (devtools_delegate_)
    devtools_delegate_->Stop();
  browser_context_.reset();
  off_the_record_browser_context_.reset();
}

}  // namespace